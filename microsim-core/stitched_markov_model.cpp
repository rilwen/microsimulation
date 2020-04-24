/*
* (C) Averisera Ltd 2017
*/
#include "stitched_markov_model.hpp"
#include "core/discrete_distribution.hpp"
#include "core/log.hpp"
#include "core/index_iterator.hpp"
#include "core/preconditions.hpp"
#include <cassert>
#include <numeric>

namespace averisera {
	namespace microsim {
		template <class S> StitchedMarkovModel<S>::StitchedMarkovModel(const state_type dim,
			const std::vector<Eigen::MatrixXd>& intra_model_transition_matrices,
			const std::vector<Eigen::MatrixXd>& inter_model_transition_matrices,
			const Eigen::VectorXd& initial_state_probs,
			const std::vector<time_type>& model_lengths) :
			dim_(dim),
			nbr_models_(intra_model_transition_matrices.size()),
			intra_model_transition_matrices_(intra_model_transition_matrices),
			intra_model_transition_cdfs_(intra_model_transition_matrices),
			inter_model_transition_matrices_(inter_model_transition_matrices),
			inter_times_intra_model_transition_cdfs_(inter_model_transition_matrices),
			initial_state_distribution_(initial_state_probs),
			initial_state_cdf_(initial_state_probs),
			cum_model_lengths_(model_lengths) {
			check_that(dim > 0, "Dimension must be positive");
			check_that(nbr_models_ > 0, "Need at least 1 model");
			check_equals(inter_model_transition_matrices.size() + 1, nbr_models_, "Incorrect number of inter-model transition matrices");
			check_equals(model_lengths.size() + 1, nbr_models_, "Incorrect number of model lengths");
			std::partial_sum(initial_state_cdf_.data(), initial_state_cdf_.data() + initial_state_cdf_.size(), initial_state_cdf_.data());
			std::partial_sum(cum_model_lengths_.begin(), cum_model_lengths_.end(), cum_model_lengths_.begin());
			auto calc_cdfs_cols = [dim](Eigen::MatrixXd& pi) {
				check_equals(dim, pi.rows(), "Wrong number of pi_expanded matrix rows");
				check_equals(dim, pi.cols(), "Wrong number of matrix columns");
				for (state_type c = 0; c < dim; ++c) {
					auto col = pi.col(c);
					double sum = col[0];
					for (state_type r = 1; r < dim; ++r) {
						sum += col[r];
						col[r] = sum;
					}
					check_that(std::abs(1.0 - col[dim - 1]) < 1e-6, "Distribution probabilities do not add to 1");
					col[dim - 1] = 1.0;
				}
			};
			for (size_t i = 0; i < nbr_models_ - 1; ++i) {
				inter_times_intra_model_transition_cdfs_[i] = inter_times_intra_model_transition_cdfs_[i] * intra_model_transition_cdfs_[i];
			}
			std::for_each(intra_model_transition_cdfs_.begin(), intra_model_transition_cdfs_.end(), calc_cdfs_cols);
			std::for_each(inter_times_intra_model_transition_cdfs_.begin(), 
				inter_times_intra_model_transition_cdfs_.end(), calc_cdfs_cols);
		}

		template <class S> StitchedMarkovModel<S> StitchedMarkovModel<S>::ordinal(state_type dim,
			const std::vector<Eigen::MatrixXd>& intra_model_transition_matrices,
			const std::vector<Eigen::VectorXd>& initial_state_distributions,
			const std::vector<time_type>& model_lengths) {
			check_that(!initial_state_distributions.empty(), "Need at least 1 initial distribution");
			const size_t nbr_models = intra_model_transition_matrices.size();
			check_equals(nbr_models, initial_state_distributions.size(), "Vector size mismatch");
			check_equals(model_lengths.size() + 1, nbr_models, "Incorrect number of model lengths");
			const auto& initial_state_probs = initial_state_distributions[0];
			check_that(nbr_models > 0, "StitchedMarkovModel::ordinal: nbr_models is 0");
			Eigen::VectorXd final_state_distribution;
			std::vector<Eigen::MatrixXd> inter_model_transition_matrices(nbr_models - 1);
			std::vector<double> prev_cdf(dim);
			std::vector<double> next_cdf(dim);
			for (size_t i = 0; i < nbr_models - 1; ++i) {
				final_state_distribution = initial_state_distributions[i];
				for (size_t k = 0; k < model_lengths[i]; ++k) {
					final_state_distribution = intra_model_transition_matrices[i] * final_state_distribution;
				}
				// map final_state_distribution to initial_state_distributions[i + 1] using percentile-to-percentile mapping
				const auto& next_initial_distr = initial_state_distributions[i + 1];
				prev_cdf[0] = final_state_distribution[0];
				next_cdf[0] = next_initial_distr[0];
				for (state_type k = 1; k < dim; ++k) {
					prev_cdf[k] = prev_cdf[k - 1] + final_state_distribution[k];
					next_cdf[k] = next_cdf[k - 1] + next_initial_distr[k];
				}
				prev_cdf.back() = 1.0;
				next_cdf.back() = 1.0;
				auto& pi = inter_model_transition_matrices[i];
				percentile_to_percentile(prev_cdf, next_cdf, pi);
			}
			return StitchedMarkovModel<S>(dim, intra_model_transition_matrices, inter_model_transition_matrices, initial_state_probs, model_lengths);
		}

		template <class S> void StitchedMarkovModel<S>::precalculate_state_distributions(const time_type cache_size) {
			state_probs_cache_.resize(dim(), cache_size);
			state_cdfs_cache_.resize(dim(), cache_size);
			if (cache_size) {
				for (size_t t = 0; t < cache_size; ++t) {
					state_probs_cache_.col(t) = calc_state_distribution_no_cache(t);
				}
				state_cdfs_cache_.row(0) = state_probs_cache_.row(0);
				for (state_type k = 1; k < dim(); ++k) {
					state_cdfs_cache_.row(k) = state_cdfs_cache_.row(k - 1) + state_probs_cache_.row(k);
				}
			}
		}

		template <class S> void StitchedMarkovModel<S>::percentile_to_percentile(const std::vector<double>& prev_cdf, const std::vector<double>& next_cdf, Eigen::MatrixXd& pi) {
			const size_t dim = prev_cdf.size();
			pi.setZero(dim, dim);
			for (state_type k = 0; k < dim; ++k) {
				auto pi_k = pi.col(k);
				const double p0 = k > 0 ? prev_cdf[k - 1] : 0.0;
				const double p1 = prev_cdf[k];
				assert(p0 <= p1);
				const double dp = p1 - p0;
				if (dp != 0) {
					const size_t l0 = DiscreteDistribution::draw_from_cdf(next_cdf.begin(), next_cdf.end(), p0);
					const size_t l1 = DiscreteDistribution::draw_from_cdf(next_cdf.begin(), next_cdf.end(), p1);

					assert(l0 <= l1);
					assert(l0 >= 0);
					assert(l1 < dim);
					assert(next_cdf[l0] >= p0);
					assert(next_cdf[l1] >= p1);

					double p = p0;
					for (size_t l = l0; l < l1; ++l) {
						const double q = next_cdf[l];
						assert(q >= p);
						pi_k[l] = q - p;
						p = q;
					}
					assert(p <= p1);
					pi_k[l1] = p1 - p;
					pi_k /= pi_k.sum();
				} else {
					const size_t l = DiscreteDistribution::draw_from_cdf(next_cdf.begin(), next_cdf.end(), p0);
					pi_k[l] = 1.0;
				}	
			}
		}

		template <class S> StitchedMarkovModel<S>::StitchedMarkovModel(const StitchedMarkovModel& other) :
			dim_(other.dim_),
			nbr_models_(other.nbr_models_),
			intra_model_transition_matrices_(other.intra_model_transition_matrices_),
			intra_model_transition_cdfs_(other.intra_model_transition_cdfs_),
			inter_model_transition_matrices_(other.inter_model_transition_matrices_),
			inter_times_intra_model_transition_cdfs_(other.inter_times_intra_model_transition_cdfs_),
			initial_state_distribution_(other.initial_state_distribution_),
			initial_state_cdf_(other.initial_state_cdf_),
			cum_model_lengths_(other.cum_model_lengths_),
			state_probs_cache_(other.state_probs_cache_),
			state_cdfs_cache_(other.state_cdfs_cache_) {
			LOG_TRACE() << "StitchedMarkovModel: copied";
		}

		template <class S> S StitchedMarkovModel<S>::draw_initial_state(double u) const {
			assert((u >= 0.) && (u <= 1.));
			return static_cast<S>(DiscreteDistribution::draw_from_cdf(initial_state_cdf_.data(),
				initial_state_cdf_.data() + initial_state_cdf_.size(), u));
		}

		template <class S> const Eigen::MatrixXd& StitchedMarkovModel<S>::get_transition_cdf_matrix(time_type t) const {
			const size_t model_idx = calc_model_index(t);
			assert(model_idx < nbr_models_);
			if (model_idx == nbr_models_ - 1) {
				// no further transitions
				return intra_model_transition_cdfs_[model_idx];
			} else {
				assert(cum_model_lengths_[model_idx] > t);
				if (t + 1 < cum_model_lengths_[model_idx]) {
					return intra_model_transition_cdfs_[model_idx];
				} else {
					// if next state belong to different model, we need to transition to the new distribution
					return inter_times_intra_model_transition_cdfs_[model_idx];
				}
			}
		}

		template <class S> S StitchedMarkovModel<S>::draw_next_state(const state_type k,
			const typename StitchedMarkovModel<S>::time_type t, const double u) const {
			assert((u >= 0.) && (u <= 1.));
			const auto& cum_pi = get_transition_cdf_matrix(t);
			return draw_transition(cum_pi.col(k), u);
		}

		template <class S> std::pair<S, double> StitchedMarkovModel<S>::draw_next_state_and_percentile(const state_type k, const time_type t, const double u) const {
			assert((u >= 0.) && (u <= 1.));
			const auto& cum_pi = get_transition_cdf_matrix(t);
			const auto cum_pi_k = cum_pi.col(k);
			const auto l = draw_transition(cum_pi_k, u); // next state
			const auto next_cdf = calc_state_cdf(t + 1); // use it to calculate the next percentile from u
			// u -> percentile mapping: rescale u from [a, b) range to [c, d)
			const double a = l > 0 ? cum_pi_k[l - 1] : 0.0;
			const double b = cum_pi_k[l];
			const double c = l > 0 ? next_cdf[l - 1] : 0.0;
			const double d = next_cdf[l];
			//TRACE() << "l=" << l << ", a=" << a << ", u=" << u << ", b=" << b;
			assert(u >= a);
			assert(u <= b);
			const double r = (a != b) ? (u - a) / (b - a) : 0.5;
			const double p = std::max(0., std::min(1., c + (d - c) * r)); // avoid rounding errors
			return std::make_pair(l, p);
		}

		template <class S> template <class V> typename StitchedMarkovModel<S>::state_type StitchedMarkovModel<S>::draw_transition(const V& distr, double u) {
			// assumes column-major storage
			return static_cast<S>(DiscreteDistribution::draw_from_cdf(distr.data(),
				distr.data() + distr.size(), u));
		}

		template <class S> size_t StitchedMarkovModel<S>::calc_model_index(time_type t) const {
			const auto model_it = std::upper_bound(cum_model_lengths_.begin(), cum_model_lengths_.end(), t);
			return std::distance(cum_model_lengths_.begin(), model_it);
		}

		template <class S> Eigen::VectorXd StitchedMarkovModel<S>::calc_state_distribution(const time_type t) const {
			if (t < cache_size()) {
				return state_probs_cache_.col(t);
			} else {
				return calc_state_distribution_no_cache(t);
			}
		}

		template <class S> Eigen::VectorXd StitchedMarkovModel<S>::calc_state_distribution_no_cache(time_type t) const {
			// TODO: use the last cached distribution if possible
			const size_t model_idx = calc_model_index(t);
			Eigen::VectorXd distr(initial_state_distribution_);
			time_type s = 0;
			for (size_t k = 0; k <= model_idx; ++k) {
				if (k > 0) {
					distr = inter_model_transition_matrices_[k - 1] * distr;
				}
				const auto& pi = intra_model_transition_matrices_[k];
				const time_type u = k < model_idx ? cum_model_lengths_[k] : t;
				for (; s < u; ++s) {
					distr = pi * distr;
				}
			}
			return distr;
		}

		template <class S> Eigen::VectorXd StitchedMarkovModel<S>::calc_state_cdf(time_type t) const {
			if (t < cache_size()) {
				return state_cdfs_cache_.col(t);
			} else {
				Eigen::VectorXd cdf(calc_state_distribution(t));
				for (state_type k = 1; k < dim_; ++k) {
					cdf[k] += cdf[k - 1];
				}
				if (dim_) {
					cdf[dim_ - 1] = 1.0;
				}
				return cdf;
			}
		}

		template class StitchedMarkovModel<uint8_t>;
		template class StitchedMarkovModel<uint16_t>;
		template class StitchedMarkovModel<uint32_t>;
	}
}

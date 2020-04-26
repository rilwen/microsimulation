// (C) Averisera Ltd 2014-2020
#include "csm_utils.hpp"
#include "markov.hpp"
#include "matrix_power_cache.hpp"
#include "preconditions.hpp"
#include "transition_matrix_inversion.hpp"

namespace averisera {
	namespace CSMUtils {
		void extrapolate(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, Eigen::MatrixXd& extrap_probs)
		{
			std::vector<Eigen::MatrixXd> v_pi(1, pi);
			std::vector<size_t> Ts(1, extrap_probs.cols());
			extrapolate(v_pi, q0, Ts, extrap_probs);
		}

		void extrapolate(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, const double time0, const std::vector<double>& extrap_times, Eigen::MatrixXd& extrap_probs) {
			assert(static_cast<size_t>(extrap_probs.cols()) == extrap_times.size());
			const size_t state_dim = q0.size();
			const size_t dim = extrap_probs.rows();
			Eigen::VectorXd state(state_dim); // copy the init. state
			state = q0;
			Eigen::VectorXd tmp(state_dim); // temp vector for updated state
			double prev_time = time0;
			MatrixPowerCache<Eigen::MatrixXd> power_cache(pi);
			for (int t = 0; t < extrap_probs.cols(); ++t) {// t is not unsigned to avoid compiler warnings about comparison between signed and unsigned int (probs.cols() is signed).
				const double next_time = extrap_times[t];
				if (prev_time != next_time) {
					assert(prev_time != next_time);
					if (next_time < time0 || next_time < prev_time || prev_time < time0) { // Always extrapolate from q0 or pi^q*q0 for q > 0, and always prefer to extrapolate forward than backward
						prev_time = time0;
						state = q0;
					}
					const bool extrapolating_forward = next_time >= prev_time;
					const unsigned int nmult = static_cast<unsigned int>(std::abs(next_time - prev_time));
					if (extrapolating_forward) {
						for (unsigned int i = 0; i < nmult; ++i) {
							// apply transition matrix
							tmp = pi * state;
							state = tmp;
						}
					} else {
						// solve the linear equation system pi^nmult * x = state for x
						tmp = TransitionMatrixInversion::apply_inverse_pi(power_cache.power(nmult), state, 1E-12);
						tmp /= tmp.sum();
						state = tmp;
					}
					prev_time = next_time;
				} else {
					assert(prev_time == next_time);
				}
				//std::cout << "--> tmp1: " << tmp1 << "\n";
				// reduce to prob. distribution for t
				CSMUtils::reduce(state.data(), state_dim, extrap_probs.col(t).data(), dim, 0);
			}
		}

		void extrapolate(const std::vector<Eigen::MatrixXd>& pi, const Eigen::VectorXd& q0, const std::vector<size_t>& segment_lengths, Eigen::MatrixXd& extrap_probs)
		{
			const size_t n_pis = pi.size();
			check_that(n_pis > 0);
			check_equals(n_pis, segment_lengths.size());
			const auto distr_dim = static_cast<unsigned int>(extrap_probs.rows());
			// Extrapolate from 1st state.
			// If we extrapolate t = 0, we extrapolate the first (historical or fitted) distribution; if we extrapolate t = probs.cols()-1, we extrapolate the last (latest) hist. distr.
			const size_t state_dim = q0.size();
			Eigen::VectorXd tmp1(state_dim); // copy the init. state
			tmp1 = q0;
			Eigen::VectorXd tmp2(state_dim); // temp vector for updated state
			size_t pi_idx = 0;
			size_t seg_len = segment_lengths.front();
			reduce(tmp1.data(), state_dim, extrap_probs.col(0).data(), distr_dim, 0); // reduce to prob. distribution for t = 0; Eigen doesn't like passing q0 to reduce, so we pass its copy from tmp1
			size_t inner_idx = 1;
			const size_t extrap_T = extrap_probs.cols();
			for (size_t t = 1; t < extrap_T; ++t) {
				check_that(seg_len > 0);
				if (pi_idx < n_pis - 1) {
					if (inner_idx == seg_len) {
						++pi_idx;
						inner_idx = 0;
						seg_len = segment_lengths[pi_idx];
					}
					++inner_idx;
				}
				// apply transition matrix
				tmp2 = pi[pi_idx] * tmp1;
				tmp1 = tmp2;

				// reduce to prob. distribution for t
				reduce(tmp1.data(), state_dim, extrap_probs.col(t).data(), distr_dim, 0);
			}
		}

		std::vector<Eigen::MatrixXd> to_hierarchical_compact_form(const Eigen::MatrixXd& pi_compact, const std::vector<unsigned int>& dimensions, const unsigned int memory) {
			const size_t ndims = dimensions.size();
			const unsigned int tot_dim = std::accumulate(dimensions.begin(), dimensions.end(), 1, std::multiplies<unsigned int>());
			const unsigned int tot_state_dim = Markov::calc_state_dim(tot_dim, memory);
			check_equals(tot_state_dim, pi_compact.cols());
			check_equals(tot_dim, pi_compact.rows());
			std::vector<Eigen::MatrixXd> result(ndims);
			unsigned int tot_known_dim = 1;
			std::vector<unsigned int> known_dims;
			known_dims.reserve(ndims);
			for (size_t i = 0; i < ndims; ++i) {
				Eigen::MatrixXd& pi = result[i];
				const unsigned int dim_i = dimensions[i];
				pi.resize(dim_i, tot_state_dim * tot_known_dim);
				pi.setZero();
				assert(tot_dim % tot_known_dim == 0);
				const unsigned int tot_unknown_dim = tot_dim / tot_known_dim;
				const unsigned int tot_summed_dim = tot_unknown_dim / dim_i;
				assert(tot_unknown_dim % dim_i == 0);
				for (unsigned int j = 0; j < tot_state_dim; ++j) {
					const unsigned int base_dst_col_idx = j * tot_known_dim;
					const auto src_col = pi_compact.col(j);
					for (unsigned int k = 0; k < tot_known_dim; ++k) {
						const unsigned int dst_col_idx = base_dst_col_idx + k;
						const unsigned int base_src_row_idx = k * tot_unknown_dim;
						auto dst_col = pi.col(dst_col_idx);
						for (unsigned int l = 0; l < tot_unknown_dim; ++l) {
							const unsigned int src_row_idx = base_src_row_idx + l;
							const unsigned int dst_row_idx = l / tot_summed_dim;
							dst_col[dst_row_idx] += src_col[src_row_idx];
						}
						const double sump = dst_col.sum();
						if (sump > 0) {
							dst_col /= sump;
						} else {
							dst_col.setConstant(1.0 / static_cast<double>(dst_col.size()));
						}
					}
				}
				tot_known_dim *= dim_i;
				known_dims.push_back(static_cast<size_t>(dim_i));
			}
			assert(tot_known_dim == tot_dim);
			return result;
		}

		void copy_probabilities(const Eigen::MatrixXd& pi, const unsigned int dim, const Eigen::VectorXd& q0, std::vector<double>& x, const unsigned int memory) {
			const unsigned int state_dim = Markov::calc_state_dim(dim, memory);
			check_equals(state_dim, static_cast<unsigned int>(q0.size()));
			check_equals(state_dim, static_cast<unsigned int>(pi.cols()));
			check_equals(dim, static_cast<unsigned int>(pi.rows()));
			if (x.size() != static_cast<size_t>(pi.size() + q0.size())) {
				x.resize(pi.size() + q0.size());
			}
			// copy transition matrix
			std::copy(pi.data(), pi.data() + pi.size(), x.begin());
			// copy initial state distribution
			std::copy(q0.data(), q0.data() + state_dim, x.begin() + dim * state_dim);
		}

		void increase_memory_length_in_transition_matrix(const Eigen::MatrixXd& low_memory_pi, Eigen::MatrixXd& high_memory_pi) {
			const auto dim = static_cast<unsigned int>(low_memory_pi.rows());
			const auto low_state_dim = static_cast<unsigned int>(low_memory_pi.cols());
			if (low_state_dim % dim != 0) {
				throw std::invalid_argument("Low-memory transition matrix has incorrect shape");
			}
			const auto state_dim = low_state_dim * dim;
			high_memory_pi.resize(dim, state_dim);
			// pilo contains distributions conditioned on all past states but the earliest one,
			// whose index changes lest frequently.
			auto dest = high_memory_pi.data();
			for (unsigned int i = 0; i < dim; ++i) {
				std::copy(low_memory_pi.data(), low_memory_pi.data() + low_memory_pi.size(), dest);
				dest += low_memory_pi.size();
			}
		}
	}
}
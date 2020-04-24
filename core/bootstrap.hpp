/*
(C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_BOOTSTRAP_H
#define __AVERISERA_BOOTSTRAP_H

#include "dirichlet_distribution.hpp"
#include "index_iterator.hpp"
#include "preconditions.hpp"
#include <Eigen/Core>
#include <cassert>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

// TODO: replace it with STL when possible
#include <boost/random/discrete_distribution.hpp>

namespace averisera {
	/** Used for estimating statistical error using the bootstrapping method.
	@tparam R STL-like RNG class */
	template <class R = std::mt19937> class Bootstrap {
	public:
		typedef R rng_type;
		
		Bootstrap(R& random) 
			:_random(random) {
		}

		// s: RNG seed
		Bootstrap(long s = 17)
			:_random(s) {
		}

		// Resample with replacement columns of in matrix and store them in out matrix.
		// in and out must have the same number of rows, but out can have different (nonzero) number of columns than in.
		// in: Input columns
		// out: Output columns
		void resample_with_replacement(const Eigen::MatrixXd& in, Eigen::MatrixXd& out);

		// Resample vector elements from in to out
		template <class V1, class V2> void resample_with_replacement(const V1& in, V2& out) {
			const size_t n1 = in.size();			
			const size_t n2 = out.size();
			if (n1 && n2) {
				std::uniform_int_distribution<size_t> dist(0, n1 - 1);
				for (size_t i = 0; i < n2; ++i) {
					out[i] = in[dist(_random)];
				}
			}
		}

		/** Resample vector elements from in to [out_begin, out_end) */
		template <class V1, class I2> void resample_with_replacement(const V1& in, I2 out_begin, const I2 out_end) {
			const size_t n1 = in.size();			
			if (n1) {
				std::uniform_int_distribution<size_t> dist(0, n1 - 1);
				for (; out_begin != out_end; ++out_begin) {
					*out_begin = in[dist(_random)];
				}
			}
		}

		/** Resample vector elements from in to out. Reorders elements of in randomly. */
		template <class V1, class V2> void resample_without_replacement(V1& in, V2& out) {
			const size_t n1 = in.size();
			const size_t n2 = out.size();
			check_that(n2 <= n1, "Bootstrap: cannot sample without replacement more outputs than inputs");
			if (n1 && n2) {
				for (size_t i = 0; i < n2; ++i) {
					const size_t n3 = n1 - i;
					assert(n3 > 0);
					assert(n3 <= n1);
					const size_t j = n3 - 1;
					std::uniform_int_distribution<size_t> dist(0, j);
					const size_t k = dist(_random);
					const auto tmp = in[k];
					out[i] = tmp;
					in[k] = in[j];
					in[j] = tmp;
				}
			}
		}

		/** Resample vector elements from in to [out_begin, out_end). Reorders elements of in randomly. */
		template <class V1, class I2> void resample_without_replacement(V1& in, I2 out_begin, const I2 out_end) {
			const size_t n1 = in.size();
			for (size_t i = 0; out_begin != out_end; ++i, ++out_begin) {				
				check_that(i < n1, "Bootstrap: cannot sample without replacement more outputs than inputs");				
				const size_t n3 = n1 - i;
				assert(n3 > 0);
				assert(n3 <= n1);
				const size_t j = n3 - 1;
				std::uniform_int_distribution<size_t> dist(0, j);
				const size_t k = dist(_random);
				const auto tmp = in[k];
				*out_begin = tmp;
				in[k] = in[j];
				in[j] = tmp;
			}			
		}

		// Calculate the start and end indices of the "confidence" part of the bootstrapped sample for a two-sided confidence interval (elements sorted by value).
		// nbr_total: Number of samples
		// confidence_level: Confidence level, e.g. 0.95
		static std::pair<size_t, size_t> two_sided(size_t nbr_total, double confidence_level);

		// Calculate the end index (exclusive) of the "confidence" part of the bootstrapped sample for a one-sided confidence interval (elements sorted by absolute error).
		// nbr_total: Number of samples
		// confidence_level: Confidence level, e.g. 0.95
		static size_t one_sided(size_t nbr_total, double confidence_level);

		// Resample discrete distribution dist1 using nbr_samples samples to form dist2.
		// dist1 and dist2 can be the same vector
		template <class V1, class V2> void resample_distribution(const V1& dist1, size_t nbr_samples, V2& dist2) {
			const size_t N = dist1.size();
			check_equals(N, dist2.size());
			if (nbr_samples) {				
				if (N > 1) {
					
					std::vector<double> alphas(N);
					std::transform(make_index_iterator_begin(dist1), make_index_iterator_end(dist1), alphas.begin(), [nbr_samples](double p){ return 1.0 + static_cast<double>(nbr_samples) * p; });
					DirichletDistribution dirichlet(std::move(alphas));
					std::vector<double> p;
					dirichlet.sample(_random, p);
					boost::random::discrete_distribution<size_t> distr(p);
					std::vector<size_t> counts(N);
					for (size_t i = 0; i < nbr_samples; ++i) {
						const size_t k = distr(_random);
						++counts[k];
					}
					std::transform(counts.begin(), counts.end(), make_index_iterator_begin(dist2), [nbr_samples](size_t c){return static_cast<double>(c) / static_cast<double>(nbr_samples); });
				} else if (N == 1) {
					dist2[0] = 1.0;
				}
			} else {
				for (size_t i = 0; i < N; ++i) {
					dist2[i] = dist1[i];
				}
			}
		}
	private:
		R _random; // pseudorandom number generator
	};

	template <class RNG> void Bootstrap<RNG>::resample_with_replacement(const Eigen::MatrixXd& in, Eigen::MatrixXd& out) {
		if (out.cols() > 0) {
			if (out.rows() != in.rows())
				throw std::domain_error("Different numbers of matrices' rows");
			std::uniform_int_distribution<size_t> dist(0, in.cols() - 1);

			for (long i = 0; i < out.cols(); i++) {
				out.col(i) = in.col(dist(_random));
			}
		} else {
			throw std::domain_error("Empty out matrix");
		}
	}

	template <class RNG> std::pair<size_t, size_t> Bootstrap<RNG>::two_sided(size_t nbr_total, double confidence_level) {
		const double p_min = (1 - confidence_level) / 2;
		const double p_max = 1 - p_min;
		return std::make_pair(one_sided(nbr_total, p_min), one_sided(nbr_total, p_max));
	}

	template <class RNG> size_t Bootstrap<RNG>::one_sided(size_t nbr_total, double confidence_level) {
		assert(confidence_level >= 0);
		return std::min(static_cast<size_t>(confidence_level * static_cast<double>(nbr_total) + 0.5), nbr_total - 1);
	}
}

#endif

// (C) Averisera Ltd 2018-19
#pragma once
#include "log.hpp"
#include "markov.hpp"
#include "math_utils.hpp"
#include "multi_index.hpp"
#include "preconditions.hpp"
#include "statistics.hpp"
#include <Eigen/Core>
#include <algorithm>
#include <numeric>

namespace averisera {
	/** @brief Utility functions for Cross Sectional Markov model family.
	*/
	namespace CSMUtils {

		/**
		Reduce the Markov process state distribution at (t, t-1, ..., t - memory) to the observed state distribution at t - lag. Uses pointers to memory buffers to operate on different data structures.

		@tparam Scalar Real scalar value type
		@param state[in] Markov process state distribution (joint probability distribution over 1 or more time points)
		@param state_size[in] Dimension of the state
		@param distr_begin[out] Target buffer for reduced (i.e. marginal) probability distribution (pointer to the beginning).
		@param distr_size[in] Dimension of distr
		@param lag What prob. distribution we need: lag = 0 is the latest, lag = memory is the first one.
		*/
		template <class Scalar> void reduce(const Scalar* state_distr, const size_t state_size, Scalar* const distr_begin, const size_t distr_size, const unsigned int lag)
		{
			assert(state_size % distr_size == 0);

			// We assume that the state distribution is a joint prob. distributions with elements laid out like that: [p_00, p_10, p_20, p_01, p_11, p_21, p_02, ...] for p_kl = P(X_t = k && X_t-1 = l). Easy
			// to generalize for more time periods. The latest state (X_t) changes most frequently.

			const Scalar* const state_distr_end = state_distr + state_size;
			Scalar* const distr_end = distr_begin + distr_size;

			const size_t sum_len = MathUtils::pow(distr_size, lag);
			assert(sum_len > 0);

			std::fill(distr_begin, distr_end, Scalar(0.0));
			while (state_distr != state_distr_end) {
				for (Scalar* distr = distr_begin; distr != distr_end; ++distr) {
					const Scalar* const sum_end = state_distr + sum_len;
					Scalar sum(*distr);
					for (; state_distr != sum_end; ++state_distr) {
						sum += *state_distr;
					}
					*distr = sum;
				}
			}
		}

		/** Normalize a range of probabilities, modifying it so that it sums to 1.
		@param it Iterator to the beginning of the range
		@param end Iterator pointing immediately beyond the range (like .end() in std::vector)
		@return Difference between sum of original probabilities and 1
		*/
		template <class Scalar, class Iter> Scalar normalize_distribution(Iter it, const Iter end) {
			const Scalar sum = std::accumulate(it, end, Scalar(0.0));
			if (sum > 0.0) {
				std::transform(it, end, it, [sum](const Scalar& x) { return x / sum; });
			}
			return sum - 1.0;
		}

		/** Calculate the number of probabilities determining the model.
		@param dim Dimension of the measured distributions (e.g. number of BMI ranges)
		@param memory Memory length
		@return Positive integer.
		*/
		inline unsigned int calc_nbr_probs(unsigned int dim, unsigned int memory) {
			return (dim + 1) * Markov::calc_state_dim(dim, memory);
		}

		/** Normalize all probability distributions parameterising the CSM model.

		@tparam Scalar real scalar value type

		@param x[in,out] Vector with prob. distributions, one after another.
		@param state_dim Dimension of Markov process distribution.
		@param dim Dimension of observed prob. distribution. x.size() % dim == 0.
		
		@return sum ( sum probs - 1 )^2
		*/
		template <class Scalar> Scalar normalize_distributions(std::vector<Scalar>& x, const unsigned int state_dim, const unsigned int dim) {
			check_equals(x.size() % dim, 0);
			auto x_it = x.begin();
			Scalar total(0.0);
			for (unsigned int i = 0; i < state_dim; ++i) {
				const auto next_x_it = x_it + dim;
				total += pow(normalize_distribution<Scalar>(x_it, next_x_it), 2);
				x_it = next_x_it;
			}
			if (x_it != x.end()) {
				assert(x.end() == x_it + state_dim);
				total += pow(normalize_distribution<Scalar>(x_it, x.end()), 2);
			}
			return total;
		}		

		/** @brief Extrapolate fitted trends.
		Even if input probabilities had missing years, extrapolation is done for every year.
		@param pi Calibrated transition matrix (dim x state_dim).
		@param q0 Calibrated initial state
		@param extrap_probs Matrix for extrapolated probability distributions, written in columns. It is expected to have proper dimensions.
		*/
		void extrapolate(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, Eigen::MatrixXd& extrap_probs);

		/** Extrapolate fitted trends for given times only, assuming q0 is for time0.
		@param pi Calibrated transition matrix (dim x state_dim).
		@param q0 Calibrated initial state
		@param time0 Initial time (will be rounded to int)
		@param extrap_times times for extrapolation, in ascending order and all >= year0 (will be rounded to int)
		@param extrap_probs Matrix for extrapolated probability distributions, written in columns. It is expected to have proper dimensions.
		*/
		void extrapolate(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, double time0, const std::vector<double>& extrap_times, Eigen::MatrixXd& extrap_probs);

		/** Extrapolate fitted trends, using a different transition matrix for each segment (the last transition matrix is used until the end of the extrapolation).
		Even if input probabilities had missing years, extrapolation is done for every year.
		@param pi Vector of calibrated transition matrices
		@param q0 Calibrated initial state distribution
		@param segment_lengths Lengths (in periods) of segments
		@param extrap_probs Matrix for extrapolated probability distributions (marginal or joint), written in columns. It is expected to have proper dimensions.
		*/
		void extrapolate(const std::vector<Eigen::MatrixXd>& pi, const Eigen::VectorXd& q0, const std::vector<size_t>& segment_lengths, Eigen::MatrixXd& extrap_probs);		

		/** Decompose the transition matrix (in compact form) of a multi-dimensional Markov process to a hierarchy of transition matrices describing choosing the first dimension value first, then the second (conditioned on the value of the first dimension), etc...

		The Markov process dimensions are flattened in order D[0], D[1], D[2]...

		@param pi_compact Matrix of the dimension N x N^(memory + 1), where N = prod_i dimensions[i]
		@param dimensions Vector of Markov process dimensions
		@param memory Process memory
		@return Vector of transition matrices of size dimensions.size(): i-th returned matrix gives the probability distributions (in columns) for the i-th dimension based on past values (if memory > 0) and the alredy selected values of lower i dimensions.
		*/
		std::vector<Eigen::MatrixXd> to_hierarchical_compact_form(const Eigen::MatrixXd& pi_compact, const std::vector<unsigned int>& dimensions, unsigned int memory);

		/** Copy probabilities (those which can be non-zero) from transition matrix and initial state to a flat vector.
		
		Flat vector is filled by columns of pi followed by q0.
		
		@param pi[in] Transition matrix (dim x state_dim).
		@param dim[in] dimension of the observed distribution
		@param q0[in] Initial state
		@param x[out] Target vector with dimension equal to dim * state_dim + state_dim. Resized if needed.
		@param memory[in] Memory length
		*/
		void copy_probabilities(const Eigen::MatrixXd& pi, unsigned int dim, const Eigen::VectorXd& q0, std::vector<double>& x, const unsigned int memory);		

		/// Calculate relative logits from probabilities (both pi and initial state).
		template <class Prob, class Logit> void probabilities_to_relative_logits(const std::vector<Prob>& probs, std::vector<Logit>& rel_logits, const unsigned int dim, const unsigned int state_dim) {
			auto p_begin = probs.begin();
			auto x_begin = rel_logits.begin();
			for (unsigned int i = 0; i < state_dim; ++i) {
				MathUtils::probabilities_to_relative_logits(p_begin, p_begin + dim, x_begin);
				p_begin += dim;
				x_begin += dim - 1;
			}
			MathUtils::probabilities_to_relative_logits(p_begin, p_begin + state_dim, x_begin);
		}

		/// Calculate relative logits from probabilities (both pi and initial state).
		template <class Prob, class Logit> void relative_logits_to_probabilities(const std::vector<Logit>& rel_logits, std::vector<Prob>& probs, const unsigned int dim, const unsigned int state_dim) {
			auto p_begin = probs.begin();
			auto x_begin = rel_logits.begin();
			for (unsigned int i = 0; i < state_dim; ++i) {
				MathUtils::probabilities_to_relative_logits(x_begin, x_begin + dim - 1, p_begin);
				p_begin += dim;
				x_begin += dim - 1;
			}
			MathUtils::probabilities_to_relative_logits(x_begin, x_begin + state_dim, p_begin);
		}

		/** Convert a transition matrix with memory M to a one with memory M+1.
		The new transition matrix is independent of the earliest observed value.

		@param low_memory_pi Transition matrix with memory M, with dimension dim x state_dim.
		@param high_memory_pi New transition matrix with memory M+1. Resized as needed.

		@throw std::invalid_argument If low_memory_pi.cols() % low_memory_pi.rows() != 0.
		*/
		void increase_memory_length_in_transition_matrix(const Eigen::MatrixXd& low_memory_pi, Eigen::MatrixXd& high_memory_pi);
	}
}
#ifndef __AVERISERA_MS_STITCHED_MARKOV_MODEL_HPP
#define __AVERISERA_MS_STITCHED_MARKOV_MODEL_HPP
/*
* (C) Averisera Ltd 2017
*/

#include <Eigen/Core>
#include <vector>

namespace averisera {
	namespace microsim {
		/** 
		A series of Markov models active in subsequent periods, composed together by inter-model transition matrices.

		Model M_i is valid for times [T_i, T_{i+1}). At time T_{i+1}:
		1. we use the model M_i to draw the "old" state value k, conditioned on the known value j for T_i
		2. we draw the "new" state value l using the k-th column of the i-th inter-model transition matrix P_i.

		@tparam S State class (unsigned int type)
		*/
		template <class S = unsigned int> class StitchedMarkovModel {
		public:
			typedef S state_type;
			typedef size_t time_type;

			/**
			@param dim Dimension of the Markov model
			@param intra_model_transition_matrices Transition matrices within each model (length N)
			@param inter_model_transition_matrices Transition matrices between models (length N - 1)
			@param initial_state_probs Initial state probabilities for the 1st model
			@param model_lengths How many steps for each model (model_lengths[i] = T_{i+1} - T_i; length N - 1)
			*/
			StitchedMarkovModel(state_type dim,
				const std::vector<Eigen::MatrixXd>& intra_model_transition_matrices,
				const std::vector<Eigen::MatrixXd>& inter_model_transition_matrices,
				const Eigen::VectorXd& initial_state_probs,
				const std::vector<time_type>& model_lengths);

			/** 
			Stitch several models together for an ordinal random variable 
			@param intra_model_transition_matrices Transition matrices within each model (length N)
			@param initial_state_distributions Initial state distributions for each model (length N)
			@param model_lengths How many steps for each model (model_lengths[i] = T_{i+1} - T_i; length N - 1)
			*/
			static StitchedMarkovModel<S> ordinal(state_type dim,
				const std::vector<Eigen::MatrixXd>& intra_model_transition_matrices,
				const std::vector<Eigen::VectorXd>& initial_state_distributions,
				const std::vector<time_type>& model_lengths);

			/**
			Precalculate state distributions (and their CDFs) from time 0.
			@param cache_size How many
			*/
			void precalculate_state_distributions(time_type cache_size);

			static void percentile_to_percentile(const std::vector<double>& prev_cdf, const std::vector<double>& next_cdf, Eigen::MatrixXd& pi);

			//StitchedMarkovModel(StitchedMarkovModel&& other);

			StitchedMarkovModel(const StitchedMarkovModel& other);
			StitchedMarkovModel& operator=(const StitchedMarkovModel& other) = delete;

			state_type dim() const {
				return dim_;
			}

			size_t nbr_models() const {
				return nbr_models_;
			}

			/** Draw X_0
			@param u Random number drawn from U(0, 1) distribution. This number will also be the percentile of X_0 distribution
			*/
			state_type draw_initial_state(double u) const;

			/** Draw X_{t+1} conditioned on X_t = k. 
			@param u Random number drawn from U(0, 1) distribution
			*/
			state_type draw_next_state(state_type k, time_type t, double u) const;

			/** Draw X_{t+1} conditioned on X_t = k and the percentile of distribution of X_{t+1} it corresponds to.
			Assumes that X is an ordinal variable.
			@param u Random number drawn from U(0, 1) distribution
			*/
			std::pair<state_type, double> draw_next_state_and_percentile(state_type k, time_type t, double u) const;

			Eigen::VectorXd calc_state_distribution(time_type t) const;

			Eigen::VectorXd calc_state_cdf(time_type t) const;

			/** Number of state probability distributions cached */
			time_type cache_size() const {
				assert(state_probs_cache_.cols() == state_cdfs_cache_.cols());
				return static_cast<time_type>(state_probs_cache_.cols());
			}
		private:
			state_type dim_;
			size_t nbr_models_;
			std::vector<Eigen::MatrixXd> intra_model_transition_matrices_;
			std::vector<Eigen::MatrixXd> intra_model_transition_cdfs_;
			std::vector<Eigen::MatrixXd> inter_model_transition_matrices_;
			std::vector<Eigen::MatrixXd> inter_times_intra_model_transition_cdfs_; /** Product of the previous intra and inter transition matrix*/
			Eigen::VectorXd initial_state_distribution_;
			Eigen::VectorXd initial_state_cdf_;
			std::vector<time_type> cum_model_lengths_; /** Cumulative sum of model_lengths */

			// Cached state distributions
			Eigen::MatrixXd state_probs_cache_;
			Eigen::MatrixXd state_cdfs_cache_;

			size_t calc_model_index(time_type t) const;

			const Eigen::MatrixXd& get_transition_cdf_matrix(time_type t) const;

			template <class V> static state_type draw_transition(const V& conditional_cdfs, double u);

			Eigen::VectorXd calc_state_distribution_no_cache(time_type t) const;
		};
	}
}

#endif // __AVERISERA_MS_STITCHED_MARKOV_MODEL_HPP

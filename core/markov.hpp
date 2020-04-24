// (C) Averisera Ltd 2018
#pragma once
#include "observed_discrete_data.hpp"
#include <Eigen/Core>

namespace averisera {	
	/** @brief Functions related to Markov processes (incl. those with finite memory). */
	namespace Markov {
		/** Calculate transition matrix from longitudinal data.
		@param data Observed data with longitudinal trajectories.
		@param traj_times Observation times.
		@param dim Dimension of the observed process.
		@param memory Process memory.
		@param use_incomplete_data Use also incomplete transitions; for memory length M, a complete transition has M+1 past points and 1 next point in a contiguous sequence.
		@return A dim x state_dim transition matrix.
		@throw std::invalid_argument If no longitudinal data.
		*/
		Eigen::MatrixXd transition_matrix(const ObservedDiscreteData& data, unsigned int dim, unsigned int memory, const bool use_incomplete_data);

		// Calculate the dimension of the Markov process "total" state, given the dimension of the measured probability distribution and memory length.
		// dim: dimension of the observed distribution
		// memory: Memory length
		// Returns state_dim(dim, memory) := dim^(memory + 1)
		unsigned int calc_state_dim(unsigned int dim, unsigned int memory);

		/** Calculate the memory of the Markov process, given the dimension of its "total" state and the dimension of the observed state. */
		unsigned int calc_memory(unsigned int state_dim, unsigned int dim);

		/** Calculte the number of transition matrix coefficients required.
		@param dim dimension of the observed distribution
		@param memory Memory length
		@return nbr_pi_coeffs(dim, memory) = dim^(memory + 2)
		*/
		inline unsigned int nbr_pi_coeffs(unsigned int dim, unsigned int memory) {
			return dim * calc_state_dim(dim, memory);
		}

		/** Try to find the steady-state solution satisfying the equation pi*y_infty = y_infty.
		@param pi Calibrated transition matrix
		@param steady_state Steady state. Resized if necessary.
		*/
		void calc_steady_state(const Eigen::MatrixXd& pi, Eigen::VectorXd& steady_state);

		/** Calculate the number of model degrees of freedom (independent parameters).
		@param dim dimension of the measured distributions (e.g. number of BMI ranges)
		@param memory memory length
		@return Non-negative integer.
		*/
		inline unsigned int calc_dof(unsigned int dim, unsigned int memory) {
			const unsigned int state_dim = calc_state_dim(dim, memory);
			return state_dim * (dim - 1) + state_dim - 1;
		}

		/** Convert transition matrix from dim x state_dim form to state_dim x state_dim.
		Assumes column-wise storage.
		@param transition_matrix[in] Pointer to transition matrix data in dim x state_dim layout, column-wise.
		@param transition_matrix_expanded[out] Pointer to expanded transition matrix data in state_dim x state_dim layout, column-wise.
		@param dim Dimension of observed process.
		@param state_dim Dimension of Markov process.
		*/
		template <class Scalar> void expand_transition_matrix(const Scalar* transition_matrix, Scalar* transition_matrix_expanded, const unsigned int dim, const unsigned int state_dim) {
			assert(state_dim % dim == 0);
			if (state_dim == dim) {
				std::copy(transition_matrix, transition_matrix + dim * dim, transition_matrix_expanded);
			} else {
				const unsigned int unobs_state_dim = state_dim / dim;
				assert(unobs_state_dim >= dim);
				// Expand the entries of the transition matrix into a square state_dim x state_dim matrix.
				std::fill(transition_matrix_expanded, transition_matrix_expanded + state_dim * state_dim, 0);
				for (unsigned int prev_state_idx = 0; prev_state_idx < state_dim; ++prev_state_idx) {
					const unsigned int next_state_idx_0 = (prev_state_idx % unobs_state_dim) * dim;
					std::copy(transition_matrix, transition_matrix + dim, transition_matrix_expanded + next_state_idx_0);
					transition_matrix += dim;
					transition_matrix_expanded += state_dim;
				}
			}
		}
	}
}

#pragma once
/*
(C) Averisera Ltd 2018
*/
#include "csm.hpp"
#include "csm_utils.hpp"
#include "jagged_2d_array.hpp"
#include "markov.hpp"
#include "multi_index.hpp"
#include "observed_discrete_data.hpp"
#include "sacado_scalar.hpp"
#include <algorithm>
#include <vector>

namespace averisera {

	/** Workspace for CSM objective function, using nested automatically differentiable double type.

	This is a smaller version without observed data. Used mostly for testing.

	@tparam L Nesting level. CSMWorkspace<L> tracks derivatives up to and including (L+1)-th order.
	*/
	template <unsigned int L> struct CSMWorkspaceNoData {
		typedef typename NestedADouble<L>::ad_type ad_scalar_t;
		typedef typename NestedADouble<L>::value_type value_t;

		CSMWorkspaceNoData(const unsigned T, const unsigned int n_dim, const unsigned int memory)
			: dim(n_dim),
			state_dim(Markov::calc_state_dim(n_dim, memory)),
			arg_dim(CSM::calc_arg_dim(n_dim, memory)),
			pi_expanded(state_dim * state_dim),
			p_approx(n_dim * T),
			state_distr_approx(state_dim * T),
			ax(arg_dim),
			highest_pi_power(1)
		{}

		CSMWorkspaceNoData(const CSMWorkspaceNoData<L - 1>& lower)
			: dim(lower.dim),
			state_dim(lower.state_dim),
			arg_dim(lower.arg_dim),
			pi_expanded(lower.pi_expanded.size()),
			p_approx(lower.p_approx.size()),
			state_distr_approx(lower.state_distr_approx.size()),
			ax(lower.ax.size()),
			highest_pi_power(1)
		{}

		const unsigned int dim; /// Dimension of observed state.
		const unsigned int state_dim; /// Dimension of Markov state.
		const unsigned int arg_dim; /// Total number of calibrated parameters (pi and q0).
		std::vector<ad_scalar_t> pi_expanded; // expanded transition matrix (incl. zero coefficients)
		std::vector<ad_scalar_t> p_approx; // approximate probability distributions (calculated from _state_approx and reduced to time t)
		std::vector<ad_scalar_t> state_distr_approx; // approximate Markov process state distribution (calculated using pi)
		std::vector<ad_scalar_t> ax; // argument of the objective function: first for pi, then for initial state.
		std::vector<ad_scalar_t> pi_powers; // higher powers of pi matrix, calculated on demand
		unsigned int highest_pi_power; // highest pi power we have calculated		

		/// Work variable in which CSMRegulariser implementations should store the calculated value of the regularisation term.
		ad_scalar_t regularisation_term;


		/** Set calibrated parameters (pi and q0) in the workspace and return the normalisation term.

		@param x Flat vector of calibrated parameters (probabilities) with size dim * state_dim + state_dim.
		@param with_gradient Whether gradient of the loss function is required.
		@throw std::invalid_argument If x.size() != ax.size().
		*/
		ad_scalar_t set_calibrated_parameters(const std::vector<double>& x, const bool with_gradient) {
			const auto arg_dim = static_cast<unsigned int>(ax.size());
			if (x.size() != arg_dim) {
				throw std::invalid_argument("Wrong number of calibrated parameters");
			}
			const unsigned int total_nbr_tracked = with_gradient ? arg_dim : 0;

			// Copy double to ad_scalar_t.
			if (with_gradient) {
				// Track derivatives.
				auto ax_it = ax.begin();
				unsigned int running_index = 0;
				for (auto x_it = x.begin(); x_it != x.end(); ++x_it, ++ax_it) {
					assert(ax_it != ax.end());
					*ax_it = from_double<L>(total_nbr_tracked, running_index, *x_it);
				}
				assert(running_index == total_nbr_tracked);
				assert(ax_it == ax.end());
			} else {
				// Use default ad_scalar_t constructor - does not track derivatives (cheaper).
				std::copy(x.begin(), x.end(), ax.begin());
			}

			// Normalise the distributions.
			const auto normalization_term = CSMUtils::normalize_distributions(ax, state_dim, dim);

			highest_pi_power = 1;

			Markov::expand_transition_matrix(&ax[0], &pi_expanded[0], dim, state_dim);

			// const auto sum_pi = std::accumulate(x.begin(), x.begin() + dim * state_dim, 0.0);
			const auto sum_pi_expanded = std::accumulate(pi_expanded.begin(), pi_expanded.end(), ad_scalar_t(0.0));

			// Copy the provided initial state distribution q0.
			// Last _state_dim entries of ax contain the initial state.
			assert(state_dim <= arg_dim);
			std::copy(ax.begin() + dim * state_dim, ax.end(), state_distr_approx.begin());
			CSMUtils::reduce(&state_distr_approx[0], state_dim, &p_approx[0], dim, 0); // Calculate new fitted marginal distribution for time 0

			return normalization_term;
		}
	};

	/** Workspace for CSM objective function, using nested automatically differentiable double type.

	@tparam L Nesting level. CSMWorkspace<L> tracks derivatives up to and including (L+1)-th order.
	*/
	template <unsigned int L> struct CSMWorkspace: public CSMWorkspaceNoData<L> {
		typedef typename NestedADouble<L>::ad_type ad_scalar_t;
		typedef typename NestedADouble<L>::value_type value_t;

		CSMWorkspace(const std::vector<double>& n_weights, const std::vector<double>& n_p, const unsigned int T, const unsigned int n_dim, const unsigned int memory, const ObservedDiscreteData& data, const double min_time)
			: CSMWorkspaceNoData<L>(T, n_dim, memory),
			weights(n_weights),
			p(n_p),
			state_indices(memory + 1),
			state_multi_index(memory + 1, this->dim)
		{
			if (memory) {
				prev_state_distr.resize(this->state_dim);
				next_state_distr.resize(this->state_dim);
				const size_t n_traj = data.ltrajs.size();
				expanded_data.init_rectangular(n_traj, T);
				nbr_specified_states.init_rectangular(n_traj, T);
				for (size_t i = 0; i < n_traj; ++i) {
					auto expanded_trajectory = expanded_data[i];
					const auto traj = data.ltrajs[i];
					const auto traj_times = data.ltimes[i];
					ObservedDiscreteData::expand_trajectory(traj, traj_times, min_time, expanded_trajectory);
					for (unsigned int t = 0; t < T; ++t) {
						nbr_specified_states(i, t) = ObservedDiscreteData::count_specified_states(expanded_trajectory, t, memory);
					}
				}
			}
		}

		CSMWorkspace(const CSMWorkspace<L - 1>& lower)
			: CSMWorkspaceNoData<L>(lower),
			weights(lower.weights),
			p(lower.p),
			state_indices(lower.state_indices.size()),
			state_multi_index(lower.state_multi_index),
			prev_state_distr(lower.prev_state_distr.size()),
			next_state_distr(lower.next_state_distr.size()),
			expanded_data(lower.expanded_data),
			nbr_specified_states(lower.nbr_specified_states)
		{}

		CSMWorkspace(const CSMWorkspace&) = delete;
		CSMWorkspace& operator=(const CSMWorkspace&) = delete;

		const std::vector<double>& weights; // weights of probabilities for each year (from data) equal to nbr_surveys
		const std::vector<double>& p; // target probability distributions (from data)
		
		

		// Work variables for log-likelihood of longitudinal data with memory
		std::vector<size_t> state_indices; /// used to index over states
		MultiIndex state_multi_index;
		/// state distributions with probabilities not compatible with data zeroed out
		std::vector<ad_scalar_t> prev_state_distr;
		std::vector<ad_scalar_t> next_state_distr;
		/// trajectory data expanded to all time periods, with -1 denoting periods when there is no data
		Jagged2DArray<int> expanded_data;
		Jagged2DArray<ObservedDiscreteData::index_t> nbr_specified_states;		

		/** Return (pi^q)_{kl}, calculating it on the fly if necessary.
		@param q number of steps
		@param k next state
		@param l prev state
		*/
		ad_scalar_t pi_power(unsigned int q, unsigned int k, unsigned int l) {
			assert(q > 0);
			if (q == 1) {
				return this->pi_expanded[l * this->state_dim + k];
			} else {
				if (q > this->highest_pi_power) {
					this->pi_powers.resize((q - 1) * this->state_dim * this->state_dim);
					for (unsigned int q2 = this->highest_pi_power + 1; q2 <= q; ++q2) {
						assert(q2 >= 2);

						const unsigned int left_power = q2 / 2;
						assert(left_power >= 1);
						const unsigned int right_power = q2 - left_power;
						assert(right_power >= 1);

						// pi^q2 = pi^left_power * pi^right_power

						const auto* right = get_pi_power(right_power);
						const auto* left = get_pi_power(left_power);
						auto* result = get_pi_power_higher(q2);
						for (unsigned int l = 0; l < this->state_dim; ++l) {
							auto* result_col = result + l * this->state_dim;
							ad_scalar_t sum(0.0);
							for (unsigned int k = 0; k < this->state_dim; ++k) {
								ad_scalar_t tmp(0.0);
								const auto* right_col = right + l * this->state_dim;
								for (unsigned int m = 0; m < this->state_dim; ++m) {
									tmp += left[m * this->state_dim + k] * right_col[m];
								}
								result_col[k] = tmp;
								sum += tmp;
							}
							if (sum != 1.0) {
								std::transform(result_col, result_col + this->state_dim, result_col, [&sum](const ad_scalar_t& p) {return p / sum; });
							}
						}
					}
					this->highest_pi_power = q;
				}

				return get_pi_power_elem(get_pi_power_higher(q), k, l);
			}
		}

	private:
		const ad_scalar_t& get_pi_power_elem(ad_scalar_t* pi_q, unsigned int k, unsigned int l) const {
			assert(k < this->state_dim);
			assert(l < this->state_dim);
			// pi_powers contains pi^2, pi^3, ...
			return pi_q[l * this->state_dim + k];
		}

		// q > 1
		ad_scalar_t* get_pi_power_higher(unsigned int q) {
			assert(q > 1);
			return &(this->pi_powers[(q - 2) * this->state_dim * this->state_dim]);
		}

		// q > 1
		const ad_scalar_t* get_pi_power_higher(unsigned int q) const {
			assert(q > 1);
			return &(this->pi_powers[(q - 2) * this->state_dim * this->state_dim]);
		}

		// q > 0
		const ad_scalar_t* get_pi_power(unsigned int q) const {
			assert(q > 0);
			return q > 1 ? &(this->pi_powers[(q - 2) * this->state_dim * this->state_dim]) : &(this->pi_expanded[0]);
		}
	};
}
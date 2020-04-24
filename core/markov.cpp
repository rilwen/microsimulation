#include "log.hpp"
#include "markov.hpp"
#include "math_utils.hpp"
#include "multi_index.hpp"
#include <Eigen/Eigenvalues>
#include <cassert>

namespace averisera {
	namespace Markov {

		static void mark_compatible_states(const std::vector<int>& expanded_trajectory, const unsigned int dim, const unsigned int memory, const unsigned int t, std::vector<bool>& flags) {
			MultiIndex mi(memory + 1, dim);
			assert(mi.flat_size() == flags.size());
			std::fill(flags.begin(), flags.end(), false);
			for (auto fit = flags.begin(); fit != flags.end(); ++fit, ++mi) {
				*fit = ObservedDiscreteData::state_index_compatible_with_data(expanded_trajectory, mi.indices(), memory, t);
			}
		}

		// Not optimized for speed
		Eigen::MatrixXd transition_matrix(const ObservedDiscreteData& data, const unsigned int dim, const unsigned int memory, const bool use_incomplete_data) {

			if (!data.ltrajs.size()) {
				throw std::invalid_argument("Markov::transition_matrix: No longitudinal data");
			}

			const unsigned int state_dim = calc_state_dim(dim, memory);

			Eigen::MatrixXd pi(dim, state_dim);
			pi.setZero();

			const double min_time = ObservedDiscreteData::first_time(data);
			const double max_time = ObservedDiscreteData::last_time(data);

			const unsigned int T = static_cast<unsigned int>(max_time - min_time + 1);
			std::vector<int> expanded_trajectory(T);
			std::vector<bool> compatible_prev_states(state_dim);

			const auto rend = data.ltrajs.row_end();
			auto time_rit = data.ltimes.row_begin();
			// First time index to check.
			const unsigned int t_min = (use_incomplete_data ? 0 : memory) + 1;
			if (t_min < T) {

				for (auto rit = data.ltrajs.row_begin(); rit != rend; ++rit, ++time_rit) {
					const auto row = *rit;
					const auto time_row = *time_rit;
					assert(static_cast<unsigned int>(row.size()) == static_cast<unsigned int>(time_row.size()));
					if (t_min) {
						ObservedDiscreteData::expand_trajectory(row, time_row, min_time, expanded_trajectory);

						for (unsigned int t = t_min; t < T; ++t) {
							const int next_observed_value = expanded_trajectory[t]; // Value observed at t.
							if (next_observed_value >= 0) {
								mark_compatible_states(expanded_trajectory, dim, memory, t - 1, compatible_prev_states);
								const size_t nbr_compatible_prev_states = std::count_if(compatible_prev_states.begin(), compatible_prev_states.end(), [](bool flag) {return flag; });
								assert(nbr_compatible_prev_states);
								if (nbr_compatible_prev_states < state_dim) {
									// Only update the matrix if there is some information in the data.
									if (use_incomplete_data || nbr_compatible_prev_states == 1) {
										const double weight = 1 / static_cast<double>(nbr_compatible_prev_states);
										for (unsigned int prev_state_idx = 0; prev_state_idx < state_dim; ++prev_state_idx) {
											if (compatible_prev_states[prev_state_idx]) {
												pi(next_observed_value, prev_state_idx) += weight;
												//LOG_DEBUG() << "Trajectory " << std::distance(data.ltrajs.row_begin(), rit) << ": pi(" << next_observed_value << ", " << prev_state_idx << ") + " << weight << " -> " << pi(next_observed_value, prev_state_idx);
											}
										}
									}									
								}								
							}
						}											
					}
				}
				assert(time_rit == data.ltimes.row_end());
			}

			for (unsigned int k = 0; k < state_dim; ++k) {
				const double sum = pi.col(k).sum();
				if (sum) {
					pi.col(k) /= sum;
				} else {
					pi.col(k).fill(1.0 / dim);
				}
			}

			return pi;
		}

		unsigned int calc_state_dim(unsigned int dim, unsigned int memory) {
			return static_cast<unsigned int>(MathUtils::pow(dim, memory + 1));
		}

		unsigned int calc_memory(unsigned int state_dim, unsigned int dim) {
			assert(state_dim % dim == 0);
			unsigned int memory = 0;
			while (state_dim > dim) {
				state_dim /= dim;
				++memory;
			}
			return memory;
		}

		void calc_steady_state(const Eigen::MatrixXd& pi, Eigen::VectorXd& steady_state)
		{
			// diagonalise pi
			Eigen::EigenSolver<Eigen::MatrixXd> es(pi, true);

			// find the eigenvalue closest to 1.0
			double min_away_from_1 = 1000000;
			unsigned int min_away_from_1_idx = 0;
			for (int i = 0; i < pi.rows(); ++i) {
				const double away_from_1 = std::abs(es.eigenvalues()[i] - 1.0);
				if (away_from_1 < min_away_from_1) {
					min_away_from_1 = away_from_1;
					min_away_from_1_idx = i;
				}
			}
			if (min_away_from_1 > 1E-4) {
				LOG_ERROR() << "Could not find a good approximation of steady state";
			}

			// use this eigenvector as a steady state
			steady_state.resize(pi.rows());

			// make all probabilities positive
			steady_state = es.eigenvectors().col(min_away_from_1_idx).cwiseAbs();

			// normalize
			const double sum = steady_state.sum();
			if (sum != 0) {
				steady_state /= sum;
			}
		}
	}
}
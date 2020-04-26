// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MS_MARKOV_MODEL_H
#define __AVERISERA_MS_MARKOV_MODEL_H

#include "core/dates_fwd.hpp"
#include "core/period.hpp"
#include <Eigen/Core>
#include <utility>
#include <vector>

namespace averisera {
    namespace microsim {
        /** @brief Markov model

        Markov model without memory, with different time step lengths for transitions from different states.

        Handles relative risks which skew the transition probabilities.
        */
        class MarkovModel {
        public:
			typedef unsigned int state_type;
            /** Construct the model
            @param[in] transition_matrix Transition matrix with conditional transition probabilities in columns. I.e. transition_matrix(j, i) = P(X_{t+1} == j | X_i).
            @param[in] transition_periods Lengths of each transition depending on the starting state; terminal states can have transition period == Period() but others must have non-zero period length
            @param[in] initial_state_probs Probability distribution for the initial state
            @throw std::domain_error If transition_matrix is empty, not normalised or not square. If transition_periods.size() != transition_matrix.rows(). If transition matrix columns are not good probability distributions. If initial_state_probs is not a good probability distribution. If initial_state_probs.size() != transition_matrix.rows().
            */
            MarkovModel(const Eigen::MatrixXd& transition_matrix, const std::vector<Period>& transition_periods, const Eigen::VectorXd& initial_state_probs);

			/** Constructor which moves the arguments. All parameter checking the same as in the copying constructor. 
			If constructor throws, parameters are not modified. */
			MarkovModel(Eigen::MatrixXd&& transition_matrix, std::vector<Period>&& transition_periods, Eigen::VectorXd&& initial_state_probs);

            /** Simple case with 1D transition periods and initial state always 0 */
            MarkovModel(const Eigen::MatrixXd& transition_matrix);

            /** Dimension of the model */
            state_type dim() const {
                return static_cast<state_type>(_transition_matrix.rows());
            }
            
            /** Get transition period
             * @param[in] from Initial state, < dim()
             */
            Period transition_period(state_type from) const {
                return _transition_periods[from];
            }

			bool is_terminal(state_type st) const {
				return transition_period(st).size == 0;
			}
            
            /** Apply relative risk to the next state distribution.
             * @param[in] from Initial state
             * @param[in] relative_risks Vector of relative risks, with NaN meaning that this probability should be untouched
             * @param[out] transition_probs Vector for probabilities after application
             * @throw std::domain_error If from >= dim(), vectors have size != dim() or relative_risks[i] < 0 for any i
             * @throw std::runtime_error If it is impossible to rescale probabilities according to relative risk values
             */
            void apply_relative_risks(state_type from, const std::vector<double>& relative_risks, std::vector<double>& transition_probs) const;
            
            /** Given the current state, select the next state taking relative risks into account.
             * @param[in] current_state Current state
             * @param[in] relative_risks Vector of relative risks, with NaN meaning that this probability should be untouched
             * @param[in] random01 Random number drawn from U(0, 1) distribution
             * @return (period, state) pair
             * @throw std::domain_error If current_state >= dim(), relative_risks.size() != dim(), relative_risks[i] < 0 for any i, or random01 outside [0, 1]
             */
            state_type select_next_state(state_type current_state, const std::vector<double>& relative_risks, double random01) const;

            /** Select new state without relative risks */
            state_type select_next_state(state_type current_state, double random01) const;

            /** Select the initial state taking relative risks into account.
             * @param[in] relative_risks Vector of relative risks, with NaN meaning that this probability should be untouched
             * @param[in] random01 Random number drawn from U(0, 1) distribution
             * @return (period, state) pair
             * @throw std::domain_error If relative_risks.size() != dim(), relative_risks[i] < 0 for any i or random01 outside [0, 1]
             */              
            state_type select_initial_state(const std::vector<double>& relative_risks, double random01) const;
            
            /** Given the current date and state, select the next date and state, taking relative risks into account.
             * @param[in] current_state (date, state) pair
             * @param[in] relative_risks Vector of relative risks, with NaN meaning that this probability should be untouched
             * @param[in] random01 Random number drawn from U(0, 1) distribution
             * @return (date, state) pair
             * @throw std::domain_error If current_state.second >= dim(), relative_risks.size() != dim(), relative_risks[i] < 0 for any i or random01 outside [0, 1]
             */
            std::pair<Date, state_type> select_next_state(const std::pair<Date, state_type>& current_state, const std::vector<double>& relative_risks, double random01) const;

            bool operator==(const MarkovModel& other) const;

            const Eigen::VectorXd& initial_state_distribution() const {
                return _initial_state_probs;
            }
        private:
            template <class V> static void apply_relative_risks_impl(const V& base_probs, const std::vector<double>& relative_risks, std::vector<double>& transition_probs);
            state_type select_state(std::vector<double>& transition_probabilities, double random01) const;
            
            Eigen::MatrixXd _transition_matrix;
            std::vector<Period> _transition_periods;
            Eigen::VectorXd _initial_state_probs;
        };
    }
}

#endif // __AVERISERA_MS_MARKOV_MODEL_H

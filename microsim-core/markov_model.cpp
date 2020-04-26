// (C) Averisera Ltd 2014-2020
#include "markov_model.hpp"
#include "core/dates.hpp"
#include "core/discrete_distribution.hpp"
#include "core/math_utils.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace averisera {
    namespace microsim {
		static void validate(const Eigen::MatrixXd& transition_matrix, const std::vector<Period>& transition_periods, const Eigen::VectorXd& initial_state_probs) {
			if (transition_matrix.rows() == 0) {
				throw std::domain_error("MarkovModel: empty transition matrix");
			}
			if (transition_matrix.rows() != transition_matrix.cols()) {
				throw std::domain_error("MarkovModel: transition matrix is not square");
			}
			if (MathUtils::safe_cast<MarkovModel::state_type>(transition_matrix.rows()) != MathUtils::safe_cast<MarkovModel::state_type>(transition_periods.size())) {
				throw std::domain_error("MarkovModel: bad number of transition periods");
			}
			if (MathUtils::safe_cast<MarkovModel::state_type>(transition_matrix.rows()) != MathUtils::safe_cast<MarkovModel::state_type>(initial_state_probs.size())) {
				throw std::domain_error("MarkovModel: bad number of initial state probabilities");
			}
			size_t i = 0;
			for (const Period& tp : transition_periods) {
				if (tp.size < 0) {
					throw std::domain_error("MarkovModel: transition period cannot have negative size");
				} else if (tp.size == 0) {
					if (transition_matrix(i, i) != 1.0) {
						throw std::domain_error("MarkovModel: non-terminal state has zero transition period");
					}
				}
				++i;
			}
			for (decltype(transition_matrix.cols()) j = 0; j < transition_matrix.cols(); ++j) {
				const Eigen::MatrixXd::ConstColXpr col = transition_matrix.col(j);
				const double sum_p = col.sum();
				if (col.minCoeff() < 0 || col.maxCoeff() > 1) {
					throw std::domain_error("MarkovModel: probabilities outside bounds");
				}
				if (std::abs(sum_p - 1) > 1E-14) {
					throw std::domain_error("MarkovModel: probability distribution not normalized");
				}
			}
			if (initial_state_probs.minCoeff() < 0 || initial_state_probs.maxCoeff() > 1) {
				throw std::domain_error("MarkovModel: initial state probabilities outside bounds");
			}
			if (std::abs(initial_state_probs.sum() - 1) > 1E-14) {
				throw std::domain_error("MarkovModel: initial state probability distribution not normalized");
			}
		}

        MarkovModel::MarkovModel(const Eigen::MatrixXd& transition_matrix, const std::vector<Period>& transition_periods, const Eigen::VectorXd& initial_state_probs)
            : _transition_matrix(transition_matrix), _transition_periods(transition_periods), _initial_state_probs(initial_state_probs) {
			validate(transition_matrix, transition_periods, initial_state_probs);
        }

		MarkovModel::MarkovModel(Eigen::MatrixXd&& transition_matrix, std::vector<Period>&& transition_periods, Eigen::VectorXd&& initial_state_probs) {
			validate(transition_matrix, transition_periods, initial_state_probs);
			_transition_matrix = std::move(transition_matrix);
			_transition_periods = std::move(transition_periods);
			_initial_state_probs = std::move(initial_state_probs);
		}

        static Eigen::VectorXd always_zero(size_t dim) {
            Eigen::VectorXd probs(dim);
            probs.setZero();
            probs[0] = 1;
            return probs;
        }

        MarkovModel::MarkovModel(const Eigen::MatrixXd& transition_matrix)
            : MarkovModel(transition_matrix, std::vector<Period>(transition_matrix.rows(), Period::days(1)), always_zero(transition_matrix.rows())) {
        }
        
        void MarkovModel::apply_relative_risks(state_type from, const std::vector<double>& relative_risks, std::vector<double>& transition_probs) const {
            if (from >= dim()) {
                throw std::domain_error("MarkovModel: initial state out of bounds");
            }
            if (static_cast<state_type>(relative_risks.size()) != dim()) {
                throw std::domain_error("MarkovModel: bad size of relative risks vector");
            }
            if (relative_risks.size() != transition_probs.size()) {
                throw std::domain_error("MarkovModel: bad size of transition probs vector");
            }
            apply_relative_risks_impl(_transition_matrix.col(from), relative_risks, transition_probs);
        }

        template <class V> void MarkovModel::apply_relative_risks_impl(const V& base_probs, const std::vector<double>& relative_risks, std::vector<double>& transition_probs) {
            double sum_base = 0;
            double sum_scaled = 0;
            const state_type dim = static_cast<state_type>(base_probs.size());
            for (state_type i = 0; i < dim; ++i) {
                const double rr = relative_risks[i];
                if (std::isnan(rr)) {
                    transition_probs[i] = base_probs[i];
                } else {                    
                    if (rr < 0) {
                        throw std::domain_error("MarkovModel: negative relative risk");
                    }
                    const double bp = base_probs[i];
                    assert(bp >= 0);
                    assert(bp <= 1);
                    sum_base += bp;
                    const double tp = bp * rr;
                    assert(tp >= 0);
                    sum_scaled += tp;
                    transition_probs[i] = tp;
                }
            }
            assert(sum_base >= 0);
            assert(sum_scaled >= 0);
            if (sum_scaled > 0) {
                for (state_type i = 0; i < dim; ++i) {
                    if (!std::isnan(relative_risks[i])) {
                        transition_probs[i] *= sum_base / sum_scaled;
                    }
                }
            } else {
                if (sum_base > 0) {
                    throw std::runtime_error("MarkovModel: impossible to apply relative risks");
                }
            }
        }
        
		MarkovModel::state_type MarkovModel::select_next_state(const state_type current_state, const std::vector<double>& relative_risks, double random01) const {
            if (random01 < 0 || random01 > 1) {
                throw std::domain_error("MarkovModel: random number outside [0, 1]");
            }
            std::vector<double> tp(dim());      
            apply_relative_risks(current_state, relative_risks, tp);
            return select_state(tp, random01);
        }

		MarkovModel::state_type MarkovModel::select_next_state(state_type current_state, double random01) const {
			const auto& base_probs = _transition_matrix.col(current_state);
			const state_type dim = static_cast<state_type>(base_probs.size());
			std::vector<double> tp(dim);
			for (state_type i = 0; i < dim; ++i) {
				tp[i] = base_probs[i];
			}
			return select_state(tp, random01);
        }

		MarkovModel::state_type MarkovModel::select_initial_state(const std::vector<double>& relative_risks, double random01) const {
            if (random01 < 0 || random01 > 1) {
                throw std::domain_error("MarkovModel: random number outside [0, 1]");
            }
            std::vector<double> tp(dim());      
            if (relative_risks.size() != tp.size()) {
                throw std::domain_error("MarkovModel: bad size of transition probs vector");
            }
            apply_relative_risks_impl(_initial_state_probs, relative_risks, tp);
            return select_state(tp, random01);
        }

		MarkovModel::state_type MarkovModel::select_state(std::vector<double>& tp, double random01) const {
            std::partial_sum(tp.begin(), tp.end(), tp.begin());
            //tp.back() = 1.0; // fix numerical issues
			return static_cast<state_type>(DiscreteDistribution::draw_from_cdf(tp.begin(), tp.end(), random01));
        }
        
        std::pair<Date, MarkovModel::state_type> MarkovModel::select_next_state(const std::pair<Date, state_type>& current_state, const std::vector<double>& relative_risks, double random01) const {
            const state_type next_state = select_next_state(current_state.second, relative_risks, random01);
            const Date next_date = current_state.first + transition_period(current_state.second);
            return std::make_pair(next_date, next_state);
        }

        bool MarkovModel::operator==(const MarkovModel& other) const {
            return _transition_matrix == other._transition_matrix && _transition_periods == other._transition_periods;
        }
    }
}

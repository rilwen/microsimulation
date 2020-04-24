#include "anchored_hazard_curve.hpp"
#include "hazard_curve.hpp"
#include "hazard_model.hpp"
#include "relative_risk_value.hpp"
#include "core/log.hpp"
#include "core/math_utils.hpp"
#include <cassert>
//#include <cmath>
#include <stdexcept>
#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace averisera {
    namespace microsim {
		static Date validate(const std::vector<std::shared_ptr<const AnchoredHazardCurve>>& curves, const std::vector<unsigned int>& next_states) {
			if (curves.empty()) {
				throw std::domain_error("HazardModel: no curves");
			}
			if (curves.size() != next_states.size()) {
				throw std::domain_error("HazardMode: vector size mismatch");
			}
			const unsigned int dim = MathUtils::safe_cast<unsigned int>(next_states.size());
			for (unsigned int next : next_states) {
				if (next >= dim) {
					throw std::domain_error("HazardMode: next state outside bounds");
				}
			}
			Date start(Date::NAD);
			for (const std::shared_ptr<const AnchoredHazardCurve>& curve_ptr : curves) {
				if (curve_ptr) {
					if (start.is_special()) {
						start = curve_ptr->start();
					} else {
						if (curve_ptr->start() != start) {
							throw std::domain_error("HazardModel: curves must have common start date");
						}
					}
				}
			}
			if (start.is_special()) {
				throw std::domain_error("HazardModel: no valid curves");
			}
			return start;
		}

        HazardModel::HazardModel(const std::vector<std::shared_ptr<const AnchoredHazardCurve>>& curves, const std::vector<unsigned int>& next_states) {
			_start = validate(curves, next_states);
			_curves = curves;
			_next_states = next_states;
        }

		HazardModel::HazardModel(std::vector<std::shared_ptr<const AnchoredHazardCurve>>&& curves, std::vector<unsigned int>&& next_states) {
			_start = validate(curves, next_states);
			_curves = std::move(curves);
			_next_states = std::move(next_states);
		}

		HazardModel::HazardModel(std::shared_ptr<const AnchoredHazardCurve> curve)
			: HazardModel(std::vector<std::shared_ptr<const AnchoredHazardCurve>>({ curve, nullptr }),
				std::vector<unsigned int>({ 1, 1 })) {}

        HazardModel::HazardModel(const HazardModel& other)
            : _curves(other._curves), _start(other._start), _next_states(other._next_states) {
        }

        HazardModel::HazardModel(HazardModel&& other) noexcept
            : _curves(std::move(other._curves)), _start(other._start), _next_states(std::move(other._next_states)) {
            other._curves.resize(0);
            other._next_states.resize(0);
            other._start = Date::NAD;
        }

        template <class T> double HazardModel::calc_transition_probability_impl(unsigned int from, Date start, Date end, const T& param) const {
            if (start < _start || end < start) {
				LOG_ERROR() << "HazardModel: period " << start << " to " << end << " outside supported range " << _start << " to INF";
                throw std::out_of_range("HazardModel: bad dates");
            }
            if (from >= dim()) {
                throw std::out_of_range("HazardModel: initial state out of range");
            }
            if (_curves[from] && _next_states[from] != from) {
                return _curves[from]->conditional_jump_probability(start, end, param);
            } else {
                return 0.;
            }
        }

        double HazardModel::calc_transition_probability(unsigned int from, Date start, Date end, const HazardRateMultiplier& hazard_rate_multiplier) const {
            return calc_transition_probability_impl(from, start, end, hazard_rate_multiplier);
        }

        double HazardModel::calc_transition_probability(unsigned int from, Date start, Date end, const std::vector<HazardRateMultiplier>& hazard_rate_multipliers) const {
            return calc_transition_probability_impl(from, start, end, hazard_rate_multipliers);
        }
            
        double HazardModel::calc_transition_probability(unsigned int from, Date start, Date end, const RelativeRiskValue& relative_risk_value) const {
            return calc_transition_probability_impl(from, start, end, relative_risk_value);
        }

        double HazardModel::calc_transition_probability(unsigned int from, Date start, Date end, const std::vector<RelativeRiskValue>& relative_risk_values) const {
            return calc_transition_probability_impl(from, start, end, relative_risk_values);
        }

        Date HazardModel::calc_end_date(const unsigned int from, const Date start, const double transition_probability, const HazardRateMultiplier& hazard_rate_multiplier) const {
            return calc_end_date_impl(from, start, transition_probability, hazard_rate_multiplier);
        }

        Date HazardModel::calc_end_date(const unsigned int from, const Date start, const double transition_probability, const std::vector<HazardRateMultiplier>& hazard_rate_multipliers) const {
            return calc_end_date_impl(from, start, transition_probability, hazard_rate_multipliers);
        }

        template <class H> Date HazardModel::calc_end_date_impl(const unsigned int from, const Date start, const double transition_probability, const H& hrm) const {
            if (from >= dim()) {
                throw std::out_of_range("HazardModel: initial state out of range");
            }
            if (start < _start) {
                throw std::out_of_range("HazardModel: bad start date");
            }
            if (transition_probability < 0 || transition_probability > 1) {
                throw std::out_of_range("HazardModel: probability out of range");
            }
            if (_curves[from] && _next_states[from] != from) {
                if (transition_probability == 0.0) {
                    return start;
                }
                const double expected_multiplied_ihr = HazardCurve::integrated_hazard_rate_from_jump_proba(transition_probability);
                if (expected_multiplied_ihr == 0.0) {
                    // mathematically this if condition is equivalent to the condition transition_probability == 0, but we want to catch rounding effects
                    return start;
                }
                // find the value of integrated hazard rate which after applying the hazard rate multiplier would correspond to the provided transition probability
                const double ihr = _curves[from]->divide_by_hazard_rate_multiplier(start, expected_multiplied_ihr, hrm);
                BOOST_ASSERT_MSG(ihr >= 0, boost::lexical_cast<std::string>(ihr).c_str());
                const Date d2 = _curves[from]->calc_next_date(start, ihr);
                //std::cout << "start == " << start << ", t1 == " << t1 << ", t2 == " << t2 << "\n";
                BOOST_ASSERT_MSG(d2 >= start, boost::lexical_cast<std::string>(d2).c_str());
                return d2;
            } else {
                return Date::POS_INF;
            }
        }

        HazardRateMultiplier HazardModel::calc_hazard_rate_multiplier(unsigned int from, const RelativeRiskValue& relative_risk_value) const {
            if (from >= dim()) {
                throw std::out_of_range("HazardModel: initial state out of range");
            }
            if (_curves[from] && _next_states[from] != from) {
                return _curves[from]->calc_hazard_rate_multiplier(relative_risk_value);
            } else {
                return HazardRateMultiplier();
            }
        }

        std::vector<HazardRateMultiplier> HazardModel::calc_hazard_rate_multiplier(unsigned int from, const std::vector<RelativeRiskValue>& relative_risk_values) const {
            if (from >= dim()) {
                throw std::out_of_range("HazardModel: initial state out of range");
            }
            if (_curves[from] && _next_states[from] != from) {
                return _curves[from]->calc_hazard_rate_multiplier(relative_risk_values);
            } else {
                return std::vector<HazardRateMultiplier>();
            }
        }

		HazardModel HazardModel::move(Date new_start) const {
			std::vector<std::shared_ptr<const AnchoredHazardCurve>> new_curves(_curves.size());
			std::transform(_curves.begin(), _curves.end(), new_curves.begin(), [new_start](const std::shared_ptr<const AnchoredHazardCurve>& cptr) {
				return cptr ? cptr->move(new_start) : nullptr;
			});
			std::vector<unsigned int> next_states_copy(_next_states);
			return HazardModel(std::move(new_curves), std::move(next_states_copy));
		}
    }
}

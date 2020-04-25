#ifndef __AVERISERA_MS_HAZARD_MODEL_H
#define __AVERISERA_MS_HAZARD_MODEL_H

#include <memory>
#include <vector>
#include "core/dates_fwd.hpp"
#include "hazard_rate_multiplier.hpp"

namespace averisera {
    namespace microsim {
        class AnchoredHazardCurve;
        struct RelativeRiskValue;
	
        /** @brief Models the risk of transitioning between subsequent states of some condition, e.g. a disease. 
          Can calculate the probability of transition over any period.
        */
        class HazardModel {
        public:
            /** @param[in] curves Vector of hazard curves describing the transition probabilities. Date periods are mapped
              to continuous time using Date::year_fraction. If curves[i] == nullptr, state i is absorbing. Curves must have equal start date.
              At least one cure must be non-null.
              @param[in] next_states next_states[i] is the state you can go to from state i. Absorbing states have next_state[i] == i.
              @throw std::domain_error If curves is empty, start is NaD, curves.size() != next_states.size() or any next_states[i] is outside [0, dim()-1].
            */
            HazardModel(const std::vector<std::shared_ptr<const AnchoredHazardCurve>>& curves, const std::vector<unsigned int>& next_states);

			HazardModel(std::vector<std::shared_ptr<const AnchoredHazardCurve>>&& curves, std::vector<unsigned int>&& next_states);

			/** Hazard model with a single transition from state 0 to absorbing state 1 */
			HazardModel(std::shared_ptr<const AnchoredHazardCurve> curve);

            HazardModel(const HazardModel& other);

            HazardModel(HazardModel&& other) noexcept;
            
            /** How many stages */
            unsigned int dim() const {
                return static_cast<unsigned int>(_curves.size());
            }

            /** Start of the time period covered by the model */
            Date start() const {
                return _start;
            }
	    
            /** Next state.
             * @param[in] from State, in [0, dim()).
             */
            unsigned int next_state(unsigned int from) const {
                return _next_states[from];
            }

            /** @brief Transition probability

              Calculate transition probability to next state in a given period.

              @param[in] from State at initial date
              @param[in] start Initial date
              @param[in] end Final date
              @param[in] hazard_rate_multiplier Describes how hazard rate is scaled
              @return P(state(end) = from + 1 | state(start) = from)
              @throw std::out_of_range If from >= dim(), start < start(), end < start
            */
            double calc_transition_probability(unsigned int from, Date start, Date end, const HazardRateMultiplier& hazard_rate_multiplier) const;
			double calc_transition_probability(unsigned int from, Date start, Date end) const {
				return calc_transition_probability(from, start, end, HazardRateMultiplier());
			}

            double calc_transition_probability(unsigned int from, Date start, Date end, const std::vector<HazardRateMultiplier>& hazard_rate_multipliers) const;

            /** @brief Transition probability

              Calculate transition probability to next state in a given period.

              @param[in] from State at initial date
              @param[in] start Initial date
              @param[in] end Final date
              @param[in] relative_risk_value Relative risk value
              @return P(state(end) = from + 1 | state(start) = from)
              @throw std::out_of_range If from >= dim(), start < start(), end < start
            */
            double calc_transition_probability(unsigned int from, Date start, Date end, const RelativeRiskValue& relative_risk_value) const;

            double calc_transition_probability(unsigned int from, Date start, Date end, const std::vector<RelativeRiskValue>& relative_risk_value) const;

            /**
              Calculate end date matching given transition probability
              @param[in] from State at initial date
              @param[in] start Initial date
              @param[in] transition_probability P(state(end) = from + 1 | state(start) = from)
              @param[in] hazard_rate_multiplier Describes how hazard rate is scaled
              @return If state from is absorbing, return Date::POS_INF. If transition probability is 0.0, return start.
              @throw std::out_of_range If from >= dim(), start < start() or transition_probability outside [0, 1]
            */
            Date calc_end_date(unsigned int from, Date start, double transition_probability, const HazardRateMultiplier& hazard_rate_multiplier = HazardRateMultiplier()) const;

            Date calc_end_date(unsigned int from, Date start, double transition_probability, const std::vector<HazardRateMultiplier>& hazard_rate_multipliers) const;

            /** Calculate hazard rate multiplier corresponding to relative risk in a given reference period.
              @param[in] from State at initial date
              @param[in] relative_risk_value RelativeRiskValue
              @return HazardRateMultiplier
              @throw std::out_of_range If from >= dim()
              @throw std::runtime_error If base probability is zero or relative risk times base probability equals or exceeds 1
            */
            HazardRateMultiplier calc_hazard_rate_multiplier(unsigned int from, const RelativeRiskValue& relative_risk_value) const;

            std::vector<HazardRateMultiplier> calc_hazard_rate_multiplier(unsigned int from, const std::vector<RelativeRiskValue>& relative_risk_values) const;

			/** Move the hazard model to new start date */
			HazardModel move(Date new_start) const;
        private:
            template <class T> inline double calc_transition_probability_impl(unsigned int from, Date start, Date end, const T& param) const;

            template <class H> Date calc_end_date_impl(unsigned int from, Date start, double transition_probability, const H& hrm) const;
            
            std::vector<std::shared_ptr<const AnchoredHazardCurve>> _curves;            
            Date _start;
            std::vector<unsigned int> _next_states;
        };
    }
}

#endif // __AVERISERA_MS_HAZARD_MODEL_H

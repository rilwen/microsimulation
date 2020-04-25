#ifndef __AVERISERA_MICROSIM_ANCHORED_HAZARD_CURVE_H
#define __AVERISERA_MICROSIM_ANCHORED_HAZARD_CURVE_H

#include "hazard_rate_multiplier.hpp"
#include "core/dates_fwd.hpp"
#include <memory>
#include <vector>

namespace averisera {
    class Daycount;
	struct Period;
    namespace microsim {
        class HazardCurve;
        class HazardCurveFactory;
        struct RelativeRiskValue;
        
        /** HazardCurve anchored at a particular date. */
        class AnchoredHazardCurve {
        public:
            virtual ~AnchoredHazardCurve();
            
            Date start() const {
                return _start;
            }

            const Daycount& daycount() const {
                return *_daycount;
            }

            /** Translate the curve to a new start date.
              @throw std::domain_error If new_start is not a valid date.
            */
            virtual std::unique_ptr<AnchoredHazardCurve> move(Date new_start) const = 0;

			std::unique_ptr<AnchoredHazardCurve> clone() const {
				return move(start());
			}

            /** Calculate average hazard rate from d1 00:00am to d2 00:00am.
              d1, d2 >= start()
              d2 >= d1
              @throw std::out_of_range If inputs out of accepted ranges
            */
            double average_hazard_rate(Date d1, Date d2) const;

            /** Calculate average hazard rate from d1 00:00am to d2 00:00am.
              d1, d2 >= start()
              d2 >= d1
              @throw std::out_of_range If inputs out of accepted ranges
             */
            double integrated_hazard_rate(Date d1, Date d2) const;

            /** Calculate average hazard rate from d1 00:00am to d2 00:00am, applying a hazard rate multiplier.
              d1, d2 >= start()
              d2 >= d1
              @param hazard_rate_multiplier Hazard rate multiplier
              @throw std::out_of_range If inputs out of accepted ranges
             */
            double integrated_hazard_rate(Date d1, Date d2, const HazardRateMultiplier hazard_rate_multiplier) const;

            /** Calculate average hazard rate from d1 00:00am to d2 00:00am, applying hazard rate multiplier(s).              
              d1, d2 >= start()
              d2 >= d1
              For each date, multipliers applicable to this date are applied as a product m1*m2*m3*...
              @param hazard_rate_multipliers Vector of hazard rate multiplier (can be empty).
              @throw std::out_of_range If inputs out of accepted ranges
             */
            double integrated_hazard_rate(Date d1, Date d2, const std::vector<HazardRateMultiplier>& hazard_rate_multipliers) const;
            
            /** Calculate jump probability from start() to d (00:00AM to 00:00AM).
              d >= start()
              @throw std::out_of_range If inputs out of accepted ranges
             */
            double jump_probability(Date d) const;

            /** Calculate probability of jump happening between d1 00:00AM and d2 00:00AM,
              conditional on it not happening until d1 00:00AM.
              d1, d2 >= start()
              d2 >= d1
              @param hazard_rate_multiplier Hazard rate multiplier
              @throw std::out_of_range If inputs out of accepted ranges
            */
            double conditional_jump_probability(Date d1, Date d2, const HazardRateMultiplier hazard_rate_multiplier) const;

			/** Calculate probability of jump happening between d1 00:00AM and d2 00:00AM,
			conditional on it not happening until d1 00:00AM.
			d1, d2 >= start()
			d2 >= d1
			@throw std::out_of_range If inputs out of accepted ranges
			*/
			double conditional_jump_probability(Date d1, Date d2) const;

            double conditional_jump_probability(Date d1, Date d2, const std::vector<HazardRateMultiplier>& hazard_rate_multipliers) const;

            /** Calculate probability of jump happening between d1 00:00AM and d2 00:00AM,
              conditional on it not happening until d1 00:00AM.
              d1, d2 >= start()
              d2 >= d1
              @param[in] relative_risk_value RelativeRiskValue
              @throw std::out_of_range If inputs out of accepted ranges
            */
            double conditional_jump_probability(Date d1, Date d2, const RelativeRiskValue& relative_risk_value) const {
                return conditional_jump_probability(d1, d2, calc_hazard_rate_multiplier(relative_risk_value));
            }

            double conditional_jump_probability(Date d1, Date d2, const std::vector<RelativeRiskValue>& relative_risk_values) const {
                return conditional_jump_probability(d1, d2, calc_hazard_rate_multiplier(relative_risk_values));
            }

            /** Find such d2, that integrated_hazard_rate(d1, d2) is as close as possible to ihr, but not larger.
              @param d1 >= start()
              @param ihr >= 0
              @throw std::out_of_range If inputs out of accepted ranges
              @return Date on or after d1. Returns value <= Date::MAX if there a non-zero jump risk on or after d1, or Date::POS_INF if there is none. If ihr == 0, returns d1.
            */
            Date calc_next_date(Date d1, double ihr) const;

            /** Given expected value of integrated_hazard_rate(start, X, hazard_rate_multiplier) for some unknown X, calculate the value of integrated_hazard_rate(start, X).
              @param start Date >= start()
              @param expected_multiplied_ihr integrated_hazard_rate(start, X, hazard_rate_multiplier) >= 0
              @throw std::out_of_range If inputs out of accepted ranges
             */
            double divide_by_hazard_rate_multiplier(Date start, double expected_multiplied_ihr, const HazardRateMultiplier& hazard_rate_multiplier) const;

            /** Given expected value of integrated_hazard_rate(start, X, hazard_rate_multipliers) for some unknown X, calculate the value of integrated_hazard_rate(start, X).
              @param start Date >= start()
              @param expected_multiplied_ihr integrated_hazard_rate(start, X, hazard_rate_multiplier) >= 0
              @param hazard_rate_multipliers Vector of hazard rate multiplier (can be empty).
              @throw std::out_of_range If inputs out of accepted ranges
             */
            double divide_by_hazard_rate_multiplier(Date start, double expected_multiplied_ihr, const std::vector<HazardRateMultiplier>& hazard_rate_multipliers) const;

            /** Safely divide integrated hazard rate value by its multiplier, preserving our conventions and avoiding NaNs. */
            static double safe_divide(double ihr, double multiplier);

            /** Build an AnchoredHazardCurve from probabilities measured over increasing or additive periods 
              @param periods_additive If true, then date_i = start + period_0 + ... + period_i; else date_i = start + period_i
			  @param conditional Are jump probabilitites from start of the curve or conditional on not jumping until the previous date
              @param jump_probabilities Probabilities of jump occurring before i-th date (if conditional == false) or between i-1-th (inclusive) and i-th date (exclusive) (if conditional == true)
			  @param multipliers Hazard rate multipliers to be applied to the curve, and reapplied when the curve is moved.
             */
			static std::unique_ptr<AnchoredHazardCurve> build(Date start,
				std::shared_ptr<const Daycount> daycount,
				std::shared_ptr<const HazardCurveFactory> hazard_curve_factory,
				const std::vector<Period>& periods,
				const std::vector<double>& jump_probabilities,
				bool periods_additive,
				bool conditional,
				const std::vector<HazardRateMultiplier>& multipliers);

            /** @param jump_probabilities Probabilities of jump occurring before i-th end date
			@param multipliers Hazard rate multipliers to be applied to the curve, and reapplied when the curve is moved.
             */
			static std::unique_ptr<AnchoredHazardCurve> build(Date start,
				std::shared_ptr<const Daycount> daycount,
				std::shared_ptr<const HazardCurveFactory> hazard_curve_factory,
				const std::vector<Date>& end_dates,
				const std::vector<double>& jump_probabilities,
				const std::vector<HazardRateMultiplier>& multipliers);

			/** Simple AnchoredHazardCurve 			
			*/
			static std::unique_ptr<AnchoredHazardCurve> build(Date start, std::shared_ptr<const Daycount> daycount, std::unique_ptr<HazardCurve>&& hazard_curve);

            /** Calculate hazard rate multiplier corresponding to relative risk in a given reference period.
              @param[in] relative_risk_value RelativeRiskValue
              @return HazardRateMultiplier
              @throw std::runtime_error If base probability is zero or relative risk times base probability equals or exceeds 1
            */
            HazardRateMultiplier calc_hazard_rate_multiplier(const RelativeRiskValue& relative_risk_value) const;

            std::vector<HazardRateMultiplier> calc_hazard_rate_multiplier(const std::vector<RelativeRiskValue>& relative_risk_value) const;
        protected:
            /**              
              @throw std::domain_error If start is NAD, daycount or hazard_curve is null.
             */
            AnchoredHazardCurve(Date start, std::shared_ptr<const Daycount> daycount, std::unique_ptr<const HazardCurve>&& hazard_curve);
        private:
            template <class I> double integrated_hazard_rate(Date d1, Date d2, I begin, I end) const;
            template <class I> double divide_by_hazard_rate_multiplier(Date start, double expected_multiplied_ihr, I begin, I end) const;
        private:
            Date _start;
        protected:
            std::shared_ptr<const Daycount> _daycount;
			std::unique_ptr<const HazardCurve> _hazard_curve;
        };
    }
}

#endif // __AVERISERA_MICROSIM_ANCHORED_HAZARD_CURVE_H

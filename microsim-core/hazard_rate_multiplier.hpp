#ifndef __AVERISERA_MICROSIM_HAZARD_RATE_MULTIPLIER_HPP
#define __AVERISERA_MICROSIM_HAZARD_RATE_MULTIPLIER_HPP

#include "core/dates.hpp"
#include <iosfwd>

namespace averisera {
    namespace microsim {
        /** Hazard rate multiplier used by HazardModel. Calculated from RelativeRiskValue. */
        struct HazardRateMultiplier {
            /** Construct a multiplier which acts all the time 
              @throw std::out_of_range If value < 0 or NaN
             */
            HazardRateMultiplier(double value = 1.0);

            /** Construct a multiplier which acts from d1 at 00.00am to d2 at 00.00am
              @throw std::out_of_range If value < 0 or to < from
            */
            HazardRateMultiplier(double value, Date d1, Date d2, bool movable);

            bool operator==(const HazardRateMultiplier& other) const;
            
            double value; /**< Value of the multiplier, >= 0 */
            Date from; /**< When the multiplier begins to apply (from 00.00am) */
            Date to; /**< When the multiplier stops applying (00.00am), >= from */
			bool movable; /** Whether the multiplier can be moved together with the anchored hazard curve */
        };		

		std::ostream& operator<<(std::ostream& os, const HazardRateMultiplier& hrm);
    }
}

#endif // __AVERISERA_MICROSIM_HAZARD_RATE_MULTIPLIER_HPP

#ifndef __AVERISERA_MS_RELATIVE_RISK_VALUE_H
#define __AVERISERA_MS_RELATIVE_RISK_VALUE_H

#include "core/dates.hpp"

namespace averisera {
    namespace microsim {


        /** Result of the relative risk calculation, including the data helping to use it effectively.
         */
        struct RelativeRiskValue {
            /** Type of value */
            enum class Type {
                FIXED, /**< Can be applied only in the reference period */
				MOVABLE, /** Can be moved in time but not rescaled */
                    SCALABLE /**< Can be moved and applied to periods of any size after rescaling */
                    };
            
            /*
              @throw std::domain_error If rr < 0, rr is not finite, rr is NaN, or re < rs 
            */
            RelativeRiskValue(double rr, Date rs, Date re, Type type);
            
            double relative_risk; /**< Relative risk */
            Date ref_start; /**< Start of the reference period (00:00am) */
            Date ref_end; /**< End of the reference period (00:00am) */
            Type type;
        };
    }
}

#endif // __AVERISERA_MS_RELATIVE_RISK_VALUE_H

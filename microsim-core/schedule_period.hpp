#ifndef __AVERISERA_MICROSIM_SCHEDULE_PERIOD_H
#define __AVERISERA_MICROSIM_SCHEDULE_PERIOD_H

#include "core/dates.hpp"
#include <iosfwd>
#include <memory>

namespace averisera {
    class Daycount;
    namespace microsim {
        /** @brief Schedule period type with precalculated derived quantities.
        */
        struct SchedulePeriod {
            /** Default constructor */
            SchedulePeriod()
            : days(0), fract(0) {
            }

            /** Construct period with single date */
            explicit SchedulePeriod(Date date);
            
            /** Constructor with dates.
             * 
             * @param[in] n_begin Start date
             * @param[in] n_end End date (on or after n_begin).
             @param[in] daycount Daycount used to calculate year fractions.
             */
            SchedulePeriod(Date n_begin, Date n_end, const Daycount& daycount);

            /** Constructor with dates.
             * 
             * @param[in] n_begin Start date
             * @param[in] n_end End date (on or after n_begin).
             @param[in] daycount Daycount used to calculate year fractions.
             */
            SchedulePeriod(Date n_begin, Date n_end, std::shared_ptr<const Daycount> daycount);
            
            bool operator==(const SchedulePeriod& other) const {
                return begin == other.begin && end == other.end;
            }
            
            Date begin;
            Date end;
            long days; /**< Period length in days */
            double fract; /**< Period length as year fraction */
        };

        std::ostream& operator<<(std::ostream& stream, SchedulePeriod sp);

    }
}

#endif // __AVERISERA_MICROSIM_SCHEDULE_PERIOD_H

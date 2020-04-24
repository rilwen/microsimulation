/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_SCHEDULE_DEF_H
#define __AVERISERA_MS_SCHEDULE_DEF_H

#include "core/dates.hpp"
#include "core/daycount.hpp"
#include "core/period.hpp"
#include <memory>

namespace averisera {
    namespace microsim {
        /*! \brief Defines a Schedule */
        struct ScheduleDefinition {

            /*! \param start Schedule start
              \param end Schedule end Must be >= start
              \param freq Schedule date frequency. Cannot be type NONE. Can have zero size only if start == end.
              \param daycount Daycount calculation method. If null, YEAR_FRACT is used
              \throw std::domain_error If parameters are incorrect
            */
            ScheduleDefinition(Date start, Date end, Period freq, std::shared_ptr<const Daycount> daycount = nullptr);
            
            ScheduleDefinition() {
                daycount = Daycount::YEAR_FRACT();
                frequency.size = 0;
                frequency.type = PeriodType::NONE;
            }
            
            Date start; /*!< Start of the schedule */
            Date end;/*!< End of the schedule */
            Period frequency; /*!< Frequency */
            std::shared_ptr<const Daycount> daycount; /*!< Daycount convention */
        };
    }
}

#endif // __AVERISERA_MS_SCHEDULE_DEF_H

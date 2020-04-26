// (C) Averisera Ltd 2014-2020
#include "schedule_definition.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        ScheduleDefinition::ScheduleDefinition(Date n_start, Date n_end, Period n_freq, std::shared_ptr<const Daycount> n_daycount)
            : start(n_start), end(n_end), frequency(n_freq), daycount(n_daycount) {
            if (end < start) {
                throw std::domain_error("ScheduleDefinition: start must be <= end");
            }
            if (frequency.type == PeriodType::NONE) {
                throw std::domain_error("ScheduleDefinition: frequency type cannot be NONE");
            }
            if (frequency.size == 0 && end != start) {
                throw std::domain_error("ScheduleDefinition: frequency size can only be zero if start == end");
            }
            if (!daycount) {
                daycount = Daycount::YEAR_FRACT();
            }
        }

    }
}

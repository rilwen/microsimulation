#include "schedule_period.hpp"
#include "core/daycount.hpp"
#include "core/dates.hpp"
#include "core/period.hpp"
#include <cassert>
#include <ostream>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        SchedulePeriod::SchedulePeriod(Date date) {
            begin = end = date;
            days = 0;
            fract = 0.;
        }
        
        SchedulePeriod::SchedulePeriod(Date n_begin, Date n_end, const Daycount& daycount) {
            assert(n_begin <= n_end);
            begin = n_begin;
            end = n_end;
            days = (end - begin).days();
            fract = daycount.calc(begin, end);
        }

        SchedulePeriod::SchedulePeriod(Date n_begin, Date n_end, std::shared_ptr<const Daycount> daycount) {
            assert(n_begin <= n_end);
            if (!daycount) {
                throw std::domain_error("SchedulePeriod: null daycount");
            }
            begin = n_begin;
            end = n_end;
            days = (end - begin).days();
            fract = daycount->calc(begin, end);
        }

        std::ostream& operator<<(std::ostream& stream, SchedulePeriod sp) {
            stream << "[" << sp.begin << ", " << sp.end << "]";
            return stream;
        }


    }
}

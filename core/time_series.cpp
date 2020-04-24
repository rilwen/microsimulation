/*
 * (C) Averisera Ltd 2015
 */
#include "time_series.hpp"
#include "dates.hpp"
#include <cmath>

namespace averisera {
    // explicit instantiation of popular versions
    template class TimeSeries<double, double>;
    template class TimeSeries<Date, double>;

    template <> bool is_not_a_time<double>(double time) {
        return std::fpclassify(time) == FP_NAN;
    }

    template <> bool is_not_a_time<Date>(Date time) {
        return time.is_not_a_date();
    }
}

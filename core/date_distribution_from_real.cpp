#include "date_distribution_from_real.hpp"
#include "daycount.hpp"

namespace averisera {
    DateDistributionFromReal::DateDistributionFromReal(Date start, std::shared_ptr<const Distribution> daycount_distr, std::shared_ptr<const Daycount> daycount, double reference_value)
        : GenericDistributionFromReal<Date>(daycount_distr), _start(start), _daycount(daycount), _reference_value(reference_value) {
        if (!_daycount) {
            throw std::domain_error("DateDistributionFromReal: null daycount");
        }
        if (_start.is_special()) {
            throw std::domain_error("DateDistributionFromReal: start date invalid");
        }
    }
    
    double DateDistributionFromReal::to_double(Date value) const {
        return _daycount->calc(_start, value) + _reference_value;
    }
    
    Date DateDistributionFromReal::from_double(double value) const {
        return _daycount->add_year_fraction(_start, value - _reference_value);
    }

    DateDistributionFromReal* DateDistributionFromReal::conditional(Date left, Date right) const {
        return new DateDistributionFromReal(_start,
            conditional_real_distr(left, right),
            _daycount,
            _reference_value);
    }
}

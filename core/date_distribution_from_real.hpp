#ifndef __AVERISERA_DATE_DISTRIBUTION_FROM_REAL_H
#define __AVERISERA_DATE_DISTRIBUTION_FROM_REAL_H

#include "dates.hpp"
#include "generic_distribution_from_real.hpp"

namespace averisera {
    class Daycount;
    
    /** Date distribution given by real-valued distribution P(X) and formula x = reference_value + daycount(d, start) */
    class DateDistributionFromReal: public GenericDistributionFromReal<Date> {
    public:
        /**
          @param start Reference date (corresponds to reference real value)
          @param daycount_distr Distribution of the daycount(start, x) 
          @param daycount Daycount method
          @param reference_value Corresponds to reference date start (defaults to 0.0)
          @throw If daycount_distr or daycount are null, or start is not a valid date.
        */
        DateDistributionFromReal(Date start, std::shared_ptr<const Distribution> daycount_distr, std::shared_ptr<const Daycount> daycount, double reference_value = 0.0);

        DateDistributionFromReal* conditional(Date left, Date right) const override;        
    private:
        double to_double(Date value) const override;
        Date from_double(double value) const override;

        Date _start;
        std::shared_ptr<const Daycount> _daycount;
        double _reference_value;
    };
}

#endif // __AVERISERA_DATE_DISTRIBUTION_FROM_REAL_H

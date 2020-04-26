// (C) Averisera Ltd 2014-2020
#include "generation.hpp"
#include "core/generic_distribution.hpp"
#include "core/period.hpp"
#include <boost/format.hpp>

namespace averisera {
    namespace microsim {
        Generation::Generation(Date begin, Date end, PersonAttributesDistribution attrib_distr, double prob, 
            std::shared_ptr<const GenericDistribution<Date>> dob_distr)
            : _begin(begin), _end(end), _attrib_distr(attrib_distr), _prob(prob), _dob_distr(dob_distr) {
            validate();
        }

        Generation::Generation(Date begin, Date end, PersonAttributesDistribution attrib_distr, double prob)
            : Generation(begin, end, attrib_distr, prob, std::make_shared<DateDistributionFromReal>(begin, std::make_shared<DistributionLinearInterpolated>(
            std::vector<double>({ 0.0, Daycount::DAYS_365()->calc(begin, end - Period::days(1)) }), std::vector<double>({ 1.0 })), Daycount::DAYS_365(), 0.0))
        {}

        Generation::Generation(Date begin, Date end, PersonAttributesDistribution attrib_distr, 
            std::shared_ptr<const GenericDistribution<Date>> global_dob_distr)
            : _begin(begin), _end(end), _attrib_distr(attrib_distr) {
            if (global_dob_distr) {
                const std::shared_ptr<const GenericDistribution<Date>> conditional_distr(global_dob_distr->conditional(begin, end));
                assert(conditional_distr && "Conditional distribution cannot be null");
                _dob_distr = conditional_distr;
                _prob = global_dob_distr->range_prob2(begin, end);
            } else {
                throw std::domain_error("InitialiserGenerations: global DOB distribution is null");
            }
            validate();
        }

        void Generation::validate() const {
            if (_end <= _begin) {
                throw std::domain_error("Generation: end date before begin date");
            }
            if (_prob < 0 || _prob > 1) {
                throw std::domain_error("Generation: prob outside [0, 1]");
            }
            if (!_dob_distr) {
                throw std::domain_error("Generation: DOB distribution is null");
            }
            if (_dob_distr->icdf_generic(0) < _begin) {
                throw std::domain_error("Generation: DOB distribution range exceeds lower bound");
            }
			const Date icdf1 = _dob_distr->icdf_generic(1);
            if (icdf1 >= _end) {
				throw std::domain_error(boost::str(boost::format("Generation: DOB distribution range exceeds upper bound: %s >= %s") % icdf1 % _end));
            }
        }
    }
}

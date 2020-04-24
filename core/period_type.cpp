#include "period_type.hpp"
#include <cstring>
#include <ostream>
#include <stdexcept>
#include <boost/format.hpp>

namespace averisera {
    std::ostream& operator<<(std::ostream& stream, const PeriodType& period_type) {
        switch (period_type) {
        case PeriodType::NONE:
            stream << "NONE";
            break;
        case PeriodType::DAYS:
            stream << "D";
            break;
        case PeriodType::WEEKS:
            stream << "W";
            break;
        case PeriodType::MONTHS:
            stream << "M";
            break;
        case PeriodType::YEARS:
            stream << "Y";
            break;
        default:
            throw std::logic_error("PeriodType: Unknown type");
        }
        return stream;
    }  

	PeriodType period_type_from_string(const char* str) {
		if (!strcmp(str, "NONE")) {
			return PeriodType::NONE;
		} else if (!strcmp(str, "D")) {
			return PeriodType::DAYS;
		} else if (!strcmp(str, "W")) {
			return PeriodType::WEEKS;
		} else if (!strcmp(str, "M")) {
			return PeriodType::MONTHS;
		} else if (!strcmp(str, "Y")) {
			return PeriodType::YEARS;
		} else {
			throw std::runtime_error(boost::str(boost::format("PeriodType: cannot parse string: %s") % str));
		}
	}
}

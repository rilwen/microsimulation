#include "dates.hpp"
#include "period.hpp"
#include <cstring>
#include <ostream>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/iterator.hpp>
#include <boost/lexical_cast.hpp>

namespace averisera {
	Period::Period(Date d1, Date d2)
		: type(PeriodType::DAYS) {
		if (!d1.is_special() && !d2.is_special()) {
			size = static_cast<int>((d2 - d1).days());
		} else {
			throw std::domain_error("Period: attempt to create a period from/to special date(s)");
		}
	}

	Period::Period(const char* str, std::string::size_type len) {
		if (!strcmp(str, "NONE")) {
			type = PeriodType::NONE;
			size = 0;
		} else {
			if (len == std::string::npos) {
				len = strlen(str);
			}
			if (len >= 2) {
				try {
					type = period_type_from_string(str + (len - 1));
					size = boost::lexical_cast<int>(boost::make_iterator_range(str, str + (len - 1)));
				} catch (std::exception&) {
					throw std::runtime_error(boost::str(boost::format("Period: cannot construct period from string: %s") % str));
				}
			} else {
				throw std::runtime_error(boost::str(boost::format("Period: cannot construct period from string: %s") % str));
			}
		}
	}

	long Period::days() const {
		switch (type) {
		case PeriodType::NONE:
			return 0;
		case PeriodType::DAYS:
			return size;
		case PeriodType::WEEKS:
			return 7 * size;
		case PeriodType::MONTHS:
			return 30 * size;
		case PeriodType::YEARS:
			return 365 * size;
		default:
			throw std::logic_error("Period: Unknown type");
		}
	}

	bool Period::operator==(const Period& other) const {
		return type == other.type && size == other.size;
	}

	Period Period::operator+(const Period& other) const {
		if (other.type == this->type) {
			return Period(this->type, size + other.size);
		} else if (this->type == PeriodType::WEEKS && other.type == PeriodType::DAYS) {
			return Period(PeriodType::DAYS, 7 * size + other.size);
		} else if (this->type == PeriodType::DAYS && other.type == PeriodType::WEEKS) {
			return Period(PeriodType::DAYS, size + 7 * other.size);
		} else if (this->type == PeriodType::YEARS && other.type == PeriodType::MONTHS) {
			return Period(PeriodType::MONTHS, 12 * size + other.size);
		} else if (this->type == PeriodType::MONTHS && other.type == PeriodType::YEARS) {
			return Period(PeriodType::MONTHS, size + 12 * other.size);
		} else {
			throw std::domain_error("Period: types incompatible for addition");
		}
	}

    std::ostream& operator<<(std::ostream& stream, const Period& period) {
        if (period.type != PeriodType::NONE) {
            stream << period.size << period.type;
        } else {
            stream << "NONE";
        }
        return stream;
    }
}

#ifndef __AVERISERA_PERIOD_HPP
#define __AVERISERA_PERIOD_HPP

#include "dates_fwd.hpp"
#include "period_type.hpp"
#include <iosfwd>
#include <string>

namespace averisera {

    /** @brief Period object */
    struct Period {
		typedef int size_type;

        PeriodType type; /**< Type of the period */
        size_type size; /** Size of the period (can be positive or negative) */

        Period()
            : type(PeriodType::NONE), size(0)
            {}

        Period(PeriodType new_type, size_type new_size)
            : type(new_type), size(new_size)
            {}

		Period(const Period& other)
			: type(other.type), size(other.size)
		{}

		/** Construct a period p spanning from d1 to d2 such that d2 == d1 + p. Type will be set to DAYS. 
		@throw std::domain_error If d1 or d2 is a special date (infinity or NAD) */
		Period(Date d1, Date d2);

		/** Construct from string e.g. 3M for 3 months period, or -2D for -2 days.
		@param str Null-terminated string
		@param len Length of the string. Pass std::string::npos if the length is unknown
		@throw std::runtime_error If str cannot be parsed as a period.
		*/
		Period(const char* str, std::string::size_type len = std::string::npos);

		Period(const std::string& str)
			: Period(str.c_str(), str.size()) {
		}

		Period& operator=(const Period& other) {
			type = other.type;
			size = other.size;
			return *this;
		}

        /** Approximate size of period in days
         * @throw std::logic_error If type is not known.
         */
        long days() const;

        bool operator==(const Period& other) const;

		Period operator/(int n) const {
			return Period(type, size / n);
		}

		/** Add one period to another. Not all types can be added together:
		- periods of the same type can
		- week and day periods can (result: type DAYS)
		- year and month periods can (result: type MONTHS)
		@throw std::domain_error If result is not defined (e.g. adding days to years)
		*/
		Period operator+(const Period& other) const;

        static Period years(size_type n) {
            return Period(PeriodType::YEARS, n);
        }

        static Period months(size_type n) {
            return Period(PeriodType::MONTHS, n);
        }

        static Period weeks(size_type n) {
            return Period(PeriodType::WEEKS, n);
        }

        static Period days(size_type n) {
            return Period(PeriodType::DAYS, n);
        }
    };

    std::ostream& operator<<(std::ostream& stream, const Period& period);

}

#endif // __AVERISERA_PERIOD_HPP

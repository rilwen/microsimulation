#ifndef __AVERISERA_PERIOD_TYPE_HPP
#define __AVERISERA_PERIOD_TYPE_HPP

#include <cstdint>
#include <iosfwd>

namespace averisera {
    /** @brief Period type */
    enum class PeriodType {
        NONE, /**< Unspecified period type */
        DAYS, /**< In days */
        WEEKS, /**< In weeks */
        MONTHS, /**< In months */
        YEARS /**< In years */
    };

    std::ostream& operator<<(std::ostream& stream, const PeriodType& period_type);

	PeriodType period_type_from_string(const char* str);
}

#endif // __AVERISERA_PERIOD_TYPE_HPP

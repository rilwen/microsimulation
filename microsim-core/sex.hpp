/*
  (C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_MS_SEX_H
#define __AVERISERA_MS_SEX_H

#include <cstdint>
#include <iosfwd>
#include <string>

namespace averisera {
    /** \namespace microsim
     * @brief Microsimulation code
     * 
     * Namespace with the microsimulation code.
     */
    namespace microsim {
        /** \enum Sex
         *@brief Enum values for sex (male or female). */
        enum class Sex : uint8_t {
            FEMALE = 0, /**< Female gender */
                MALE = 1 /**< Male gender */
                };

        std::ostream& operator<<(std::ostream& os, Sex sex);

        /** @throw std::runtime_error If str does not map to a valid Sex enum */
        Sex sex_from_string(const std::string& str);
    }
}

#endif // __AVERISERA_MS_SEX_H

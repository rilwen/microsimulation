// (C) Averisera Ltd 2014-2020
#include "sex.hpp"
#include <algorithm>
#include <array>
#include <stdexcept>
#include <boost/format.hpp>

namespace averisera {
    namespace microsim {

        namespace {
            static const std::array<std::string, 2> _sex_strings = {std::string("FEMALE"), std::string("MALE")};
        }
            
        std::ostream& operator<<(std::ostream& os, Sex sex) {
            os << _sex_strings[static_cast<size_t>(sex)];
            return os;
        }                
        
        Sex sex_from_string(const std::string& str) {
            const auto it = std::find(_sex_strings.begin(), _sex_strings.end(), str);
            if (it != _sex_strings.end()) {
                return static_cast<Sex>(std::distance(_sex_strings.begin(), it));
            } else {
                throw std::runtime_error(boost::str(boost::format("Invalid Sex string value: %s") % str));
            }
        }
    }
}

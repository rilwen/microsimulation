#include "core/dates.hpp"
#include "hazard_rate_multiplier.hpp"
#include <cmath>
#include <stdexcept>
#include <boost/format.hpp>

namespace averisera {
    namespace microsim {
        HazardRateMultiplier::HazardRateMultiplier(double v)
            : value(v), from(Date::NEG_INF), to(Date::POS_INF), movable(true) {
            if (value < 0) {
                throw std::out_of_range(boost::str(boost::format("HazardRateMultiplier: negative value %g") % value));
            }
            if (std::isnan(value)) {
                throw std::out_of_range("HazardRateMultiplier: value is NaN");
            }
        }
        
        HazardRateMultiplier::HazardRateMultiplier(double v, Date d1, Date d2, bool n_movable)
            : value(v), from(d1), to(d2), movable(n_movable) {
			if (d1.is_not_a_date()) {
				throw std::out_of_range("HazardRateMultiplier: D1 is NAD");
			}
			if (d2.is_not_a_date()) {
				throw std::out_of_range("HazardRateMultiplier: D2 is NAD");
			}
            if (value < 0) {
                throw std::out_of_range(boost::str(boost::format("HazardRateMultiplier: negative value %g") % value));
            }
            if (to < from) {
                throw std::out_of_range("HazardRateMultiplier: from and to dates out of order");
            }
        }

        bool HazardRateMultiplier::operator==(const HazardRateMultiplier& other) const {
			return (value == other.value)
				&& (from == other.from)
				&& (to == other.to)
				&& (movable == other.movable);
        }	

		std::ostream& operator<<(std::ostream& os, const HazardRateMultiplier& hrm) {
			os << "[" << hrm.value << ", " << hrm.from << ", " << hrm.to << "]";
			return os;
		}
    }
}

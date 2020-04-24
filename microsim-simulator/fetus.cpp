/*
(C) Averisera Ltd 2017
*/
#include "fetus.hpp"

namespace averisera {
	namespace microsim {
		std::ostream& operator<<(std::ostream& os, const Fetus& fetus) {
			os << "(" << fetus.attributes() << ", " << fetus.conception_date() << ")";
			return os;
		}
	}
}

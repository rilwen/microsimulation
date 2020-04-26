// (C) Averisera Ltd 2014-2020
#include "printable.hpp"
#include <sstream>

namespace averisera {
    Printable::~Printable() {
    }

	std::string Printable::as_string() const {
		std::stringstream ss;
		print(ss);
		return ss.str();
	}
}

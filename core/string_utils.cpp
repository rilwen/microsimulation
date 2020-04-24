/*
(C) Averisera Ltd 2017
*/
#include "string_utils.hpp"
#include <sstream>

namespace averisera {
	namespace StringUtils {
		std::string join(const std::vector<std::string>& strings, const std::string& on) {
			if (strings.empty()) {
				return std::string();
			}
			std::stringstream ss;
			auto it = strings.begin();
			ss << *it;
			++it;
			for (; it != strings.end(); ++it) {
				ss << on << *it;
			}
			return ss.str();
		}
	}
}

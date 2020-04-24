#pragma once
/*
(C) Averisera Ltd 2017
*/
#include <string>
#include <vector>

namespace averisera {
	/** String utility functions */
	namespace StringUtils {
		/** Like Python's join function */
		std::string join(const std::vector<std::string>& strings, const std::string& on);
	}
}

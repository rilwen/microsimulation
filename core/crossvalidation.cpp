/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include "crossvalidation.hpp"
#include <numeric>
#include <vector>

namespace averisera {
	namespace CrossValidation {
		void fix_years(std::vector<double>& years, size_t T) {
			if (years.empty()) {
				// provide made-up years to simplify later code
				years.resize(T);
				std::iota(years.begin(), years.end(), 0); // v[0] = 0, v[1] = 1, ...
			}			
		}
	}
}
/*
(C) Averisera Ltd 2017
*/
#include "combinatorics.hpp"
#include "math_utils.hpp"
#include <boost/math/special_functions/binomial.hpp>
#include <cassert>

namespace averisera {
	namespace Combinatorics {
		unsigned long partial_partition(unsigned int m, int n) {
			if (m == 0 && n == 0) {
				return 1;
			} else if (n <= 0 || m <= 0 || MathUtils::safe_cast<int>(m) > n) {
				return 0;
			} else if (m == 1) {
				return 1;
			} else {
				return partial_partition(m, n - m) + partial_partition(m - 1, n - 1);
			}
		}

		unsigned long partial_partition_restricted_size(unsigned int m, int n, unsigned int k) {
			if (k <= 1) {
				return partial_partition(m, n);
			} else {
				return partial_partition(m, n - (k - 1) * m);
			}
		}

		unsigned long partial_composition(unsigned int m, int n) {
			if (m == 0 && n == 0) {
				return 1;
			} else if (m <= 0 || n <= 0 || MathUtils::safe_cast<int>(m) > n) {
				return 0;
			} else {
				const double bin_coeff = boost::math::binomial_coefficient<double>(n - 1, m - 1);
				return MathUtils::safe_cast<unsigned long>(bin_coeff);
			}
		}

		unsigned long partial_composition_restricted_size(unsigned int m, int n, unsigned int k) {
			if (k <= 1) {
				return partial_composition(m, n);
			} else {
				return partial_composition(m, n - m * (k - 1));
			}
		}
	}
}

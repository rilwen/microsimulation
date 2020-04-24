/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include "math_utils.hpp"
#include "rng.hpp"
#include <cmath>
#include <iostream>
#include <boost/math/constants/constants.hpp>

namespace averisera {
	namespace MathUtils {
		size_t pow(const size_t m, const unsigned int power) {
			size_t res = 1;
			for (unsigned int i = 0; i < power; ++i) {
				res *= m;
			}
			return res;
		}

		const double pi = boost::math::constants::pi<double>();

		double random_round(const double x, RNG& rng) {
			const double c = ceil(x);
			if (x == c) {
				return x;
			}
			const double f = floor(x);
			if (x == f) {
				return x;
			}
			const double p = x - f;
			const double u = rng.next_uniform();
			return u < p ? c : f;
		}

		int solve_quadratic(double a, double b, double c, double& x1, double& x2) {
			if (a != 0) {
				const double delta = b * b - 4 * a * c;
				if (delta > 0) {
					if (b != 0) {
						const double temp = -0.5 * (b + sgn(b) * sqrt(delta));
						x1 = temp / a;
						x2 = c / temp;						
					} else {
						x1 = sqrt(delta) / 2 / a;
						x2 = -x1;
					}
					return 2;
				} else if (delta < 0) {
					return 0;
				} else {
					x1 = -b / (2 * a);
					return 1;
				}
			} else if (b != 0) {
				x1 = -c / b;
				return 1;
			} else {
				return 0;
			}
		}
	}
}

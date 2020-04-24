#pragma once
#include "preconditions.hpp"

namespace averisera {
	/** Approximates dy/dx given x and y using the Kruger algorithm:
	"Constrained Cubic Spline Interpolation for Chemical Engineering Applications" by Kruger, C. J. C.

	@tparam T Real value type
	*/
	template <class T = double> struct KrugerApproximation {
		/** @param x X values
		@param y Y values
		@param[out] dy dY/dX values
		@tparam V1 Vector-like class with [] operator and size() method 
		@tparam V2 Vector-like class with [] operator and size() method
		@tparam V3 Vector-like class with [] operator and size() method
		@throw std::domain_error If x.size() != y.size() or x.size() != dy.size()
		*/
		template <class V1, class V2, class V3> static void calculate(const V1& x, const V2& y, V3& dy) {
			const size_t n = x.size();
			check_equals(x.size(), y.size());
			check_equals(x.size(), dy.size());
			if (!n) {
				return;
			}
			if (n == 1) {
				dy[0] = 0.0;
			} else if (n == 2) {
				dy[0] = (y[1] - y[0]) / (x[1] - x[0]);
				dy[1] = dy[0];
			} else {
				T x_ip1 = x[n - 1];
				T y_ip1 = y[n - 1];
				T x_i = x[n - 2];
				T y_i = y[n - 2];
				T x_im1;
				T y_im1;
				for (size_t i = n - 2; i >= 1; --i) {
					x_im1 = x[i - 1];
					y_im1 = y[i - 1];
					if (y_i < std::min(y_im1, y_ip1) || y_i > std::max(y_im1, y_ip1)) {
						dy[i] = 0.0;
					} else {
						dy[i] = 2 / ((x_ip1 - x_i) / (y_ip1 - y_i) + (x_i - x_im1) / (y_i - y_im1));
					}
					x_ip1 = x_i;
					y_ip1 = y_i;
					x_i = x_im1;
					y_i = y_im1;
				}
				assert(y_i == y[0]);
				assert(y_ip1 == y[1]);
				assert(x_i == x[0]);
				assert(x_ip1 == x[1]);
				dy[0] = 0.5 * (3 * (y_ip1 - y_i) / (x_ip1 - x_i) - dy[1]);
				dy[n - 1] = 0.5 * (3 * (y[n - 1] - y[n - 2]) / (x[n - 1] - x[n - 2]) - dy[n - 2]);
			}
		}
	};
}

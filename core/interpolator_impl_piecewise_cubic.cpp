#include "interpolator_impl_piecewise_cubic.hpp"
#include "math_utils.hpp"

namespace averisera {
	InterpolatorImplPiecewiseCubic::InterpolatorImplPiecewiseCubic(const std::vector<DataNode>& data)
		: InterpolatorImplPiecewisePolynomial<2>(data) {
	}

	bool InterpolatorImplPiecewiseCubic::is_monotonic(size_t i, double rel_tol) const {
		assert(i < nbr_segments());
		assert(rel_tol >= 0);
		assert(rel_tol <= 1);
		const double h = x()[i + 1] - x()[i];
		std::array<double, 3> b; // derivative coefficients
		const auto& a = coeffs(i);
		b[0] = a[1];
		b[1] = 2 * a[2];
		b[2] = 3 * a[3];
		double x1, x2;
		const int nbr_roots = MathUtils::solve_quadratic(b[2], b[1], b[0], x1, x2);
		const double lb = rel_tol * h;
		const double rb = (1 - rel_tol) * h;
		switch (nbr_roots) {
		case 0:
			return true;
		case 1:
			return x1 <= lb || x1 >= rb;
		default:
			assert(nbr_roots == 2);
			return (x1 <= lb || x1 >= rb) && (x2 <= lb || x2 >= rb);
		}		
	}
}

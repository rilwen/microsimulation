#include <gtest/gtest.h>
#include "core/interpolator_impl_tanh.hpp"
#include <boost/format.hpp>
#include <cmath>

using namespace averisera;

static void test_tanh_solution(double lambda, double dx, double y0, double y1, double dydx0, double dydx1, double ax, double bx, double ay, double by, double tol = 1e-8) {
	const std::string msg(boost::str(boost::format("%g %g %g %g %g") % dx % y0 % y1 % dydx0 % dydx1));
	ASSERT_NEAR(y0, ay * tanh(bx) + by, tol) << msg;
	ASSERT_NEAR(y1, ay * tanh(ax * dx + bx) + by, tol) << msg;
	ASSERT_NEAR(dydx0 * dx, ay * ax * dx * (1 - pow(tanh(bx), 2)), tol / sqrt(lambda)) << msg;
	ASSERT_NEAR(dydx1 * dx, ay * ax * dx * (1 - pow(tanh(ax * dx + bx), 2)), tol / sqrt(lambda)) << msg;
}

TEST(InterpolatorImplTanh, calibrate_tanh_node) {
	double ax; double ay; double bx; double by;
	const double dx = 0.5;	
	const double lambda = 1;
	double y0 = 0.1; double y1 = 1.1; double dydx0 = 2.2; double dydx1 = 0.01;
	InterpolatorImplTanh::calibrate_tanh_node(lambda, dx, y0, y1, dydx0, dydx1, ax, bx, ay, by);
	test_tanh_solution(lambda, dx, y0, y1, dydx0, dydx1, ax, bx, ay, by);
	y0 = 0.1; y1 = 1.1; dydx0 = 0.01; dydx1 = 2.2;
	InterpolatorImplTanh::calibrate_tanh_node(lambda, dx, y0, y1, dydx0, dydx1, ax, bx, ay, by);
	test_tanh_solution(lambda, dx, y0, y1, dydx0, dydx1, ax, bx, ay, by);
	y0 = 1.1; y1 = 0.1; dydx0 = -0.01; dydx1 = -2.2;
	InterpolatorImplTanh::calibrate_tanh_node(lambda, dx, y0, y1, dydx0, dydx1, ax, bx, ay, by);
	test_tanh_solution(lambda, dx, y0, y1, dydx0, dydx1, ax, bx, ay, by);
	y0 = 1.1; y1 = 0.1; dydx0 = -2.2; dydx1 = -0.01;
	InterpolatorImplTanh::calibrate_tanh_node(lambda, dx, y0, y1, dydx0, dydx1, ax, bx, ay, by);
	test_tanh_solution(lambda, dx, y0, y1, dydx0, dydx1, ax, bx, ay, by);
	y0 = 1.1; y1 = 0.1; dydx0 = -0.01; dydx1 = -0.01001;
	InterpolatorImplTanh::calibrate_tanh_node(lambda, dx, y0, y1, dydx0, dydx1, ax, bx, ay, by);
	test_tanh_solution(lambda, dx, y0, y1, dydx0, dydx1, ax, bx, ay, by, 5e-8);
	y0 = 0.1; y1 = 0.1001; dydx0 = 0.01; dydx1 = 2.2;
	InterpolatorImplTanh::calibrate_tanh_node(lambda, dx, y0, y1, dydx0, dydx1, ax, bx, ay, by);
	test_tanh_solution(lambda, dx, y0, y1, dydx0, dydx1, ax, bx, ay, by, 5e-3);
}

TEST(InterpolatorImplTanh, Test) {
	const std::vector<double> x({ 0.0, 1.0, 2.2, 3.0, 4.0 });
	const std::vector<double> y({ 0.0, 0.07, 0.22, 0.24, 0.41 });
	//const double left_dydx = 0.1;
	//const double right_dydx = 5.0;
	InterpolatorImplTanh interp(std::vector<double>(x), y, 1);
	for (size_t i = 0; i < x.size(); ++i) {
		ASSERT_NEAR(y[i], interp.evaluate(x[i]), 1E-8) << i;
		if (i > 0 && i < x.size() - 1) {
			const double dl = interp.evaluate_derivative(x[i], i - 1);
			const double dr = interp.evaluate_derivative(x[i], i);
			double tol = 2e-8;
            if (1 == i) {
                tol = 0.02;
			} else if (2 == i) {
				tol = 0.01;
			} else if (3 == i) {
				tol = 0.1;
			}
			EXPECT_NEAR(dl, dr, tol) << i;
		}
	}
	/*const double dx = x.back() / 100.0;
	for (size_t i = 0; i <= 100; ++i) {
		const double xx = x.front() + i * dx;
		std::cout << xx << "\t" << interp.evaluate(xx) << "\n";
	}*/
}

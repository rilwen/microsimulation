// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/fritsch_butland_approximation.hpp"
#include "core/interpolator_impl_piecewise_cubic.hpp"
#include "core/stl_utils.hpp"
#include <Eigen/Core>
#include <array>

using namespace averisera;

TEST(FritschButlandApproximation, Test) {
	const size_t n = 10;
	std::vector<double> x(n);
	Eigen::VectorXd y(x.size());
	std::array<double, n> dy;
	std::vector<double> exact_dy(n);
	for (unsigned int i = 0; i < n; ++i)
	{
		x[i] = i;
		y[i] = cos(i*0.5);
		exact_dy[i] = -0.5*sin(0.5*i);
	}
	FritschButlandApproximation<double>::calculate(x, y, dy);
	for (unsigned int i = 0; i < n; ++i)
		EXPECT_NEAR(exact_dy[i], dy[i], 0.09);
	for (size_t i = 0; i < n; ++i) {
		y[i] = 2;
	}
	FritschButlandApproximation<double>::calculate(x, y, dy);
	for (size_t i = 0; i < n; ++i) {
		EXPECT_NEAR(0, dy[i], 1E-15);
	}
}

TEST(FritschButlandApproximation, IsMonotonic) {
	const std::vector<double> x({ 0, 0.9, 1, 1.1, 2 });
	const std::vector<double> y({ 100, 0, 200, 0, 100 });
	std::vector<double> dy(x.size());
	FritschButlandApproximation<double>::calculate(x, y, dy);
	std::vector<InterpolatorImplPiecewiseCubic::DataNode> data(x.size());
	for (size_t i = 0; i < x.size(); ++i) {
		data[i].x() = x[i];
		data[i].y() = { y[i], dy[i] };
	}
	InterpolatorImplPiecewiseCubic cubic(data);
	for (size_t i = 0; i < x.size() - 1; ++i) {
		ASSERT_TRUE(cubic.is_monotonic(i, 1e-7)) << i << " " << cubic.coeffs(i) << "\n" << dy[i] << " " << dy[i + 1];
	}
}

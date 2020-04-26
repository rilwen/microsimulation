// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/interpolator_impl_piecewise_cubic.hpp"

using namespace averisera;

TEST(InterpolatorImplPiecewiseCubic, Monotonic) {
	std::vector<InterpolatorImplPiecewiseCubic::DataNode> data(2);
	data[0].x() = 0;
	data[0].y() = { 0, 1 };
	data[1].x() = 1;
	data[1].y() = { 1, 1 };
	InterpolatorImplPiecewiseCubic cubic(data);
	ASSERT_NEAR(0.0, cubic.coeffs(0)[0], 1E-12);
	ASSERT_NEAR(1.0, cubic.coeffs(0)[1], 1E-12);
	ASSERT_NEAR(0.0, cubic.coeffs(0)[2], 1E-12);
	ASSERT_NEAR(0.0, cubic.coeffs(0)[3], 1E-12);
	ASSERT_TRUE(cubic.is_monotonic(0));
}

TEST(InterpolatorImplPiecewiseCubic, NotMonotonic) {
	std::vector<InterpolatorImplPiecewiseCubic::DataNode> data(2);
	data[0].x() = -1;
	data[0].y() = { 1, -2 };
	data[1].x() = 1;
	data[1].y() = { 1, 2 };
	InterpolatorImplPiecewiseCubic cubic(data);
	ASSERT_NEAR(1.0, cubic.coeffs(0)[0], 1E-12);
	ASSERT_NEAR(-2.0, cubic.coeffs(0)[1], 1E-12);
	ASSERT_NEAR(1.0, cubic.coeffs(0)[2], 1E-12);
	ASSERT_NEAR(0.0, cubic.coeffs(0)[3], 1E-12);
	ASSERT_FALSE(cubic.is_monotonic(0));
}

TEST(InterpolatorImplPiecewiseCubic, Monotonic2) {
	std::vector<InterpolatorImplPiecewiseCubic::DataNode> data(2);
	data[0].x() = 0;
	data[0].y() = { 100, -333.33333333333337 };
	data[1].x() = 0.9;
	data[1].y() = { 0, 0 };
	InterpolatorImplPiecewiseCubic cubic(data);
	ASSERT_FALSE(cubic.is_monotonic(0, 0));
	ASSERT_TRUE(cubic.is_monotonic(0, 1e-7));
}

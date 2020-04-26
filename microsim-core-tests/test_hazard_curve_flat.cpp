// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-core/hazard_curve/hazard_curve_flat.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(HazardCurveFlat, MultiplyHazardRate) {
	const double r = 0.2;
	const double f = 0.5;
	HazardCurveFlat curve(r);
	auto c1 = curve.multiply_hazard_rate(0, std::numeric_limits<double>::infinity(), f);
	ASSERT_NEAR(f * r, c1->instantaneous_hazard_rate(0), 1E-15);
	ASSERT_NEAR(f * r, c1->instantaneous_hazard_rate(1000), 1E-15);
	c1 = curve.multiply_hazard_rate(1, std::numeric_limits<double>::infinity(), f);
	ASSERT_NEAR(f * r, c1->instantaneous_hazard_rate(1), 1E-15);
	ASSERT_NEAR(f * r, c1->instantaneous_hazard_rate(1000), 1E-15);
	ASSERT_EQ(r, c1->instantaneous_hazard_rate(0));
	c1 = curve.multiply_hazard_rate(0, 1, f);
	ASSERT_NEAR(f * r, c1->instantaneous_hazard_rate(0), 1E-15);
	ASSERT_EQ(r, c1->instantaneous_hazard_rate(1));
	ASSERT_EQ(r, c1->instantaneous_hazard_rate(1000));
	c1 = curve.multiply_hazard_rate(1, 2, f);
	ASSERT_EQ(r, c1->instantaneous_hazard_rate(0));
	ASSERT_NEAR(f * r, c1->instantaneous_hazard_rate(1), 1E-15);
	ASSERT_EQ(r, c1->instantaneous_hazard_rate(2));
	ASSERT_EQ(r, c1->instantaneous_hazard_rate(100));
}

#include <gtest/gtest.h>
#include "microsim-core/hazard_curve.hpp"
#include "microsim-core/hazard_curve/hazard_curve_flat.hpp"
#include <cmath>
#include <limits>

using namespace averisera::microsim;

TEST(HazardCurve, StaticConversions) {
    ASSERT_NEAR(0.0, HazardCurve::jump_probability(0.0), 1E-15);
    ASSERT_NEAR(1.0, HazardCurve::jump_probability(1E12), 1E-15);
    ASSERT_NEAR(1.0, HazardCurve::jump_probability(std::numeric_limits<double>::infinity()), 1E-15);
    const double ihr = 0.1;
    const double p = HazardCurve::jump_probability(ihr);
    ASSERT_NEAR(1 - exp(-ihr), p, 1E-12);
    ASSERT_NEAR(ihr, HazardCurve::integrated_hazard_rate_from_jump_proba(p), 1E-14);
}

TEST(HazardCurve, MultiplyProbability1) {
	const double r = 0.025;
	const double pf = 0.5;
	HazardCurveFlat flat(r);
	const double t0 = 1;
	const double t1 = 3;
	const auto scaled = flat.multiply_probability(t0, t1, pf);
	ASSERT_EQ(flat.jump_probability(0, t0), scaled->jump_probability(0, t0));
	ASSERT_NEAR(pf * flat.jump_probability(t0, t1), scaled->jump_probability(t0, t1), 1E-15);
	ASSERT_EQ(flat.jump_probability(t1, 10), scaled->jump_probability(t1, 10));
}

TEST(HazardCurve, MultiplyProbability2) {
	const double r = 0.0;
	const double pf = 0.5;
	HazardCurveFlat flat(r);
	const double t0 = 1;
	const double t1 = 3;
	const auto scaled = flat.multiply_probability(t0, t1, pf);
	ASSERT_EQ(flat.jump_probability(0, t0), scaled->jump_probability(0, t0));
	ASSERT_NEAR(pf * flat.jump_probability(t0, t1), scaled->jump_probability(t0, t1), 1E-15);
	ASSERT_EQ(flat.jump_probability(t1, 10), scaled->jump_probability(t1, 10));
}

TEST(HazardCurve, MultiplyProbability3) {
	const double r = 0.025;
	const double pf = 0.5;
	HazardCurveFlat flat(r);
	const double t0 = 1;
	const double t1 = 1;
	const auto scaled = flat.multiply_probability(t0, t1, pf);
	ASSERT_EQ(flat.jump_probability(0, t0), scaled->jump_probability(0, t0));
	ASSERT_NEAR(pf * flat.jump_probability(t0, t1), scaled->jump_probability(t0, t1), 1E-15);
	ASSERT_EQ(flat.jump_probability(t1, 10), scaled->jump_probability(t1, 10));
}

#include <gtest/gtest.h>
#include "microsim-core/hazard_curve_factory.hpp"
#include "microsim-core/hazard_curve.hpp"
#include <cmath>

using namespace averisera;
using namespace averisera::microsim;

TEST(HazardCurveFactory, PiecewiseConstant) {
    std::vector<double> times({0.5, 2});
    std::vector<double> jump_probs({0.1, 0.2});
    const auto hc = HazardCurveFactory::PIECEWISE_CONSTANT()->build(times, jump_probs, false);
    ASSERT_NE(hc, nullptr);
    const double i1 = hc->integrated_hazard_rate(0, times[0]);
    ASSERT_NEAR(i1 / times[0], hc->average_hazard_rate(0, times[0]), 1E-15);
    const double i2 = hc->integrated_hazard_rate(times[0], times[1]);
    ASSERT_NEAR(i2 / (times[1] - times[0]), hc->average_hazard_rate(times[0], times[1]), 1E-15);
    ASSERT_NEAR(i1 + i2, hc->integrated_hazard_rate(0, times[1]), 1E-15);
    ASSERT_NEAR((i1 + i2) / times[1], hc->average_hazard_rate(0, times[1]), 1E-15);
    ASSERT_NEAR(exp(-i1), 1 - jump_probs[0], 1E-14);
    ASSERT_NEAR(exp(-i2 - i1), 1 - jump_probs[1], 1E-14);
}

TEST(HazardCurveFactory, PiecewiseConstantConditional) {
	const std::vector<double> times({ 0.5, 2 });
	const std::vector<double> jump_probs({ 0.1, 0.2 });
	const std::vector<double> conditional_jump_probs({ jump_probs[0], (jump_probs[1] - jump_probs[0]) / (1 - jump_probs[0]) });
	const auto hc = HazardCurveFactory::PIECEWISE_CONSTANT()->build(times, conditional_jump_probs, true);
	ASSERT_NE(hc, nullptr);
	const double i1 = hc->integrated_hazard_rate(0, times[0]);
	ASSERT_NEAR(i1 / times[0], hc->average_hazard_rate(0, times[0]), 1E-15);
	const double i2 = hc->integrated_hazard_rate(times[0], times[1]);
	ASSERT_NEAR(i2 / (times[1] - times[0]), hc->average_hazard_rate(times[0], times[1]), 1E-15);
	ASSERT_NEAR(i1 + i2, hc->integrated_hazard_rate(0, times[1]), 1E-15);
	ASSERT_NEAR((i1 + i2) / times[1], hc->average_hazard_rate(0, times[1]), 1E-15);
	ASSERT_NEAR(exp(-i1), 1 - jump_probs[0], 1E-14);
	ASSERT_NEAR(exp(-i2 - i1), 1 - jump_probs[1], 1E-14);
}

TEST(HazardCurveFactory, PiecewiseConstantZeroRisk) {
    std::vector<double> times({0.5, 2});
    std::vector<double> jump_probs({0., 0.});
    const auto hc = HazardCurveFactory::PIECEWISE_CONSTANT()->build(times, jump_probs, false);
    ASSERT_NE(hc, nullptr);
    const double i1 = hc->integrated_hazard_rate(0, times[0]);
    ASSERT_NEAR(i1, 0.0, 1E-15);
    ASSERT_NEAR(0.0, hc->average_hazard_rate(0, times[0]), 1E-15);
    const double i2 = hc->integrated_hazard_rate(times[0], times[1]);
    ASSERT_NEAR(i2, 0.0, 1E-15);
    ASSERT_NEAR(0.0, hc->average_hazard_rate(times[0], times[1]), 1E-15);
    ASSERT_NEAR(i1 + i2, hc->integrated_hazard_rate(0, times[1]), 1E-15);
    ASSERT_NEAR((i1 + i2) / times[1], hc->average_hazard_rate(0, times[1]), 1E-15);    
}

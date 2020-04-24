/*
* (C) Averisera Ltd 2015
*/
#include "microsim-core/hazard_curve/hazard_curve_piecewise_constant.hpp"
#include <gtest/gtest.h>
#include <limits>

using namespace averisera;
using namespace averisera::microsim;

TEST(HazardCurvePiecewiseConstant, Test) {
	ASSERT_THROW(new HazardCurvePiecewiseConstant(TimeSeries<double, double>()), std::domain_error);

	std::vector<double> times = { 0., 2. };
	std::vector<double> rates = { 0.1, 0.05 };
	HazardCurvePiecewiseConstant curve(TimeSeries<double, double>(times, rates));
	ASSERT_EQ(2u, curve.size());
	for (unsigned int i = 0; i < 2; ++i) {
		ASSERT_EQ(rates[i], curve.rate(i)) << i;
		ASSERT_EQ(rates[i], curve.instantaneous_hazard_rate(times[i])) << i;
		ASSERT_EQ(rates[i], curve.instantaneous_hazard_rate(times[i] + 1E-12)) << i;
		if (i > 0) {
			ASSERT_EQ(rates[i - 1], curve.instantaneous_hazard_rate(times[i] - 1E-12)) << i;
			ASSERT_NEAR(rates[i - 1], curve.average_hazard_rate(times[i - 1], times[i]), 1E-14) << i;
		}
	}
	ASSERT_NEAR(0.2, curve.integrated_hazard_rate(0, 2), 1E-14);
	ASSERT_NEAR(0.3, curve.integrated_hazard_rate(0, 4), 1E-14);
	ASSERT_NEAR(0.075, curve.average_hazard_rate(0, 4), 1E-14);
	ASSERT_NEAR(0.075, curve.average_hazard_rate(1, 3), 1E-14);
    ASSERT_NEAR(2, curve.calc_t2(0, 0.2), 1E-14);
    ASSERT_NEAR(3, curve.calc_t2(0, 0.25), 1E-14);
    ASSERT_NEAR(3, curve.calc_t2(1, 0.15), 1E-14);
    ASSERT_NEAR(1, curve.calc_t2(0, 0.1), 1E-14);
    ASSERT_NEAR(0, curve.calc_t2(0, 0), 0);
    ASSERT_NEAR(0.01, curve.calc_t2(0.01, 0), 0);

	const auto copy = curve.clone();
	ASSERT_EQ(curve.integrated_hazard_rate(0.012, 0.39), copy->integrated_hazard_rate(0.012, 0.39));
    
    auto slid = curve.slide(0.2);
    ASSERT_NEAR(rates[0], slid->instantaneous_hazard_rate(0), 1E-16);
    ASSERT_NEAR(rates[0] * 1.8 + rates[1] * 2.2, slid->integrated_hazard_rate(0, 4), 1E-14);
    slid = curve.slide(times[1]);
	ASSERT_NEAR(rates[1], slid->instantaneous_hazard_rate(0.0), 1E-16);
    ASSERT_NEAR(2 * rates[1], slid->integrated_hazard_rate(0.0, 2), 1E-16);
	slid = curve.slide(times[1] + 0.1);
	ASSERT_NEAR(rates[1], slid->instantaneous_hazard_rate(0.0), 1E-16);
    ASSERT_THROW(curve.slide(-0.01), std::out_of_range);

	times[0] = -0.01;
	ASSERT_THROW(new HazardCurvePiecewiseConstant(TimeSeries<double, double>(times, rates)), std::domain_error);
    times[0] = 0;
    rates[0] = -0.1;
    ASSERT_THROW(new HazardCurvePiecewiseConstant(TimeSeries<double, double>(times, rates)), std::domain_error);

	curve.set_rate(0, 0.025);
	ASSERT_EQ(0.025, curve.rate(0));
    ASSERT_THROW(curve.set_rate(0, -0.025), std::domain_error);
}

TEST(HazardCurvePiecewiseConstant, TestLongCurve) {
    std::vector<double> times = { 0., 2., 3., 4.5 };
    std::vector<double> rates = { 0.1, 0.05, 0.01, 0.2 };
    HazardCurvePiecewiseConstant curve(TimeSeries<double, double>(times, rates));
    double actual = curve.integrated_hazard_rate(1, 3.5);
    double expected = 0.1 + 0.05 + 0.5 * 0.01;
    ASSERT_NEAR(expected, actual, 1E-14);
    ASSERT_NEAR(3.5, curve.calc_t2(1, actual), 1E-14);
    actual = curve.integrated_hazard_rate(1, 5);
    expected = 0.1 + 0.05 + 1.5 * 0.01 + 0.5 * 0.2;
    ASSERT_NEAR(expected, actual, 1E-14);
    ASSERT_NEAR(5, curve.calc_t2(1, actual), 1E-14);
    actual = curve.integrated_hazard_rate(5, 7);
    expected = 2 * 0.2;
    ASSERT_NEAR(expected, actual, 1E-14);
    ASSERT_NEAR(7, curve.calc_t2(5, actual), 1E-14);
}

TEST(HazardCurvePiecewiseConstant, ZeroCurve) {
    std::vector<double> times = { 0. };
    std::vector<double> rates = { 0. };
    HazardCurvePiecewiseConstant curve(TimeSeries<double, double>(times, rates));
    ASSERT_EQ(0, curve.integrated_hazard_rate(1, 3.5));
    ASSERT_EQ(std::numeric_limits<double>::infinity(), curve.calc_t2(1, 0.01));
    ASSERT_EQ(1, curve.calc_t2(1, 0.));
}

TEST(HazardCurvePiecewiseConstant, TestLongCurveZeroEnd) {
    std::vector<double> times = { 0., 2., 3., 4.5 };
    std::vector<double> rates = { 0.1, 0.05, 0.01, 0. };
    HazardCurvePiecewiseConstant curve(TimeSeries<double, double>(times, rates));
    double actual = curve.integrated_hazard_rate(1, 5);
    double expected = 0.1 + 0.05 + 1.5 * 0.01;
    ASSERT_NEAR(expected, actual, 1E-14);
    ASSERT_EQ(std::numeric_limits<double>::infinity(), curve.calc_t2(1, actual + 0.1));
    ASSERT_NEAR(4.5, curve.calc_t2(1, actual), 1E-14);
}

TEST(HazardCurvePiecewiseConstant, Clone) {
    std::vector<double> times = { 0., 2., 3., 4.5 };
    std::vector<double> rates = { 0.1, 0.05, 0.01, 0. };
    HazardCurvePiecewiseConstant orig(TimeSeries<double, double>(times, rates));
    const auto copy = orig.clone();
    for (size_t i = 0; i < times.size(); ++i) {
        ASSERT_EQ(rates[i], copy->instantaneous_hazard_rate(times[i])) << i;
    }
}

TEST(HazardCurvePiecewiseConstant, MultiplyHazardRate1) {
	std::vector<double> times = { 0., 2., 3. };
	std::vector<double> rates = { 0.1, 0.05, 0.01 };
	HazardCurvePiecewiseConstant orig(TimeSeries<double, double>(times, rates));
	const double f = 0.4;
	auto c1 = orig.multiply_hazard_rate(0, std::numeric_limits<double>::infinity(), f);
	for (size_t i = 0; i < times.size(); ++i) {
		ASSERT_NEAR(f * rates[i], c1->instantaneous_hazard_rate(times[i]), 1E-15) << i;
	}
	c1 = orig.multiply_hazard_rate(2, 2, f);
	for (size_t i = 0; i < times.size(); ++i) {
		ASSERT_EQ(rates[i], c1->instantaneous_hazard_rate(times[i])) << i;
	}
	c1 = orig.multiply_hazard_rate(2, 3, 1);
	for (size_t i = 0; i < times.size(); ++i) {
		ASSERT_EQ(rates[i], c1->instantaneous_hazard_rate(times[i])) << i;
	}
}

TEST(HazardCurvePiecewiseConstant, MultiplyHazardRate2) {
	std::vector<double> times = { 0., 2., 3. };
	std::vector<double> rates = { 0.1, 0.05, 0.01 };
	HazardCurvePiecewiseConstant orig(TimeSeries<double, double>(times, rates));
	const double f = 0.4;
	auto c1 = orig.multiply_hazard_rate(0, 2, f);
	ASSERT_NEAR(rates[0] * f, c1->instantaneous_hazard_rate(0), 1E-15);
	for (size_t i = 1; i < times.size(); ++i) {
		ASSERT_EQ(rates[i], c1->instantaneous_hazard_rate(times[i])) << i;
	}
}

TEST(HazardCurvePiecewiseConstant, MultiplyHazardRate3) {
	std::vector<double> times = { 0., 2., 3. };
	std::vector<double> rates = { 0.1, 0.05, 0.01 };
	HazardCurvePiecewiseConstant orig(TimeSeries<double, double>(times, rates));
	const double f = 0.4;
	auto c1 = orig.multiply_hazard_rate(2, std::numeric_limits<double>::infinity(), f);
	ASSERT_EQ(rates[0], c1->instantaneous_hazard_rate(0));
	for (size_t i = 1; i < times.size(); ++i) {
		ASSERT_NEAR(f * rates[i], c1->instantaneous_hazard_rate(times[i]), 1E-15) << i;
	}
}

TEST(HazardCurvePiecewiseConstant, MultiplyHazardRate4) {
	std::vector<double> times = { 0., 2., 3. };
	std::vector<double> rates = { 0.1, 0.05, 0.01 };
	HazardCurvePiecewiseConstant orig(TimeSeries<double, double>(times, rates));
	const double f = 0.4;
	auto c1 = orig.multiply_hazard_rate(2, 3, f);
	ASSERT_EQ(rates[0], c1->instantaneous_hazard_rate(0));
	ASSERT_NEAR(f * rates[1], c1->instantaneous_hazard_rate(times[1]), 1E-15);
	ASSERT_EQ(rates[2], c1->instantaneous_hazard_rate(3));	
}

TEST(HazardCurvePiecewiseConstant, MultiplyHazardRate5) {
	std::vector<double> times = { 0., 2., 3. };
	std::vector<double> rates = { 0.1, 0.05, 0.01 };
	HazardCurvePiecewiseConstant orig(TimeSeries<double, double>(times, rates));
	const double f = 0.4;
	const double t0 = 2.1;
	const double t1 = 2.9;
	auto c1 = orig.multiply_hazard_rate(t0, t1, f);
	for (size_t i = 0; i < 2; ++i) {
		ASSERT_EQ(rates[i], c1->instantaneous_hazard_rate(times[i])) << i;
	}
	ASSERT_NEAR(f * rates[1], c1->instantaneous_hazard_rate(t0), 1E-15);
	ASSERT_EQ(rates[1], c1->instantaneous_hazard_rate(t1));
	ASSERT_EQ(rates[2], c1->instantaneous_hazard_rate(times[2]));
}

TEST(HazardCurvePiecewiseConstant, MultiplyHazardRate6) {
	std::vector<double> times = { 0., 2., 3. };
	std::vector<double> rates = { 0.1, 0.05, 0.01 };
	HazardCurvePiecewiseConstant orig(TimeSeries<double, double>(times, rates));
	const double f = 0.4;
	const double t0 = 1.5;
	const double t1 = 2.5;
	auto c1 = orig.multiply_hazard_rate(t0, t1, f);
	ASSERT_EQ(rates[0], c1->instantaneous_hazard_rate(0));
	ASSERT_NEAR(f * rates[0], c1->instantaneous_hazard_rate(t0), 1E-15);
	ASSERT_NEAR(f * rates[1], c1->instantaneous_hazard_rate(times[1]), 1E-15);
	ASSERT_EQ(rates[1], c1->instantaneous_hazard_rate(t1));
	ASSERT_EQ(rates[2], c1->instantaneous_hazard_rate(times[2]));
}

TEST(HazardCurvePiecewiseConstant, MultiplyHazardRate7) {
	std::vector<double> times = { 0., 2., 3. };
	std::vector<double> rates = { 0.1, 0.05, 0.01 };
	HazardCurvePiecewiseConstant orig(TimeSeries<double, double>(times, rates));
	const double f = 0.4;
	const double t0 = 1.5;
	const double t1 = 2;
	auto c1 = orig.multiply_hazard_rate(t0, t1, f);
	ASSERT_EQ(rates[0], c1->instantaneous_hazard_rate(0));
	ASSERT_NEAR(f * rates[0], c1->instantaneous_hazard_rate(t0), 1E-15);
	ASSERT_EQ(rates[1], c1->instantaneous_hazard_rate(times[1]));
	ASSERT_EQ(rates[2], c1->instantaneous_hazard_rate(times[2]));
}

TEST(HazardCurvePiecewiseConstant, MultiplyHazardRate8) {
	std::vector<double> times = { 0., 2., 3. };
	std::vector<double> rates = { 0.1, 0.05, 0.01 };
	HazardCurvePiecewiseConstant orig(TimeSeries<double, double>(times, rates));
	const double f = 0.4;
	const double t0 = 2;
	const double t1 = 2.5;
	auto c1 = orig.multiply_hazard_rate(t0, t1, f);
	ASSERT_EQ(rates[0], c1->instantaneous_hazard_rate(0));
	ASSERT_NEAR(f * rates[1], c1->instantaneous_hazard_rate(t0), 1E-15);
	ASSERT_EQ(rates[1], c1->instantaneous_hazard_rate(t1));
	ASSERT_EQ(rates[2], c1->instantaneous_hazard_rate(times[2]));
}


TEST(HazardCurvePiecewiseConstant, MultiplyHazardRate9) {
	std::vector<double> times = { 0., 2., 3. };
	std::vector<double> rates = { 0.1, 0.05, 0.01 };
	HazardCurvePiecewiseConstant orig(TimeSeries<double, double>(times, rates));
	const double f = 0.4;
	const double t0 = 2;
	const double t1 = 3.5;
	auto c1 = orig.multiply_hazard_rate(t0, t1, f);
	ASSERT_EQ(rates[0], c1->instantaneous_hazard_rate(0));
	ASSERT_NEAR(f * rates[1], c1->instantaneous_hazard_rate(t0), 1E-15);
	ASSERT_NEAR(f * rates[2], c1->instantaneous_hazard_rate(times[2]), 1E-15);
	ASSERT_EQ(rates[2], c1->instantaneous_hazard_rate(t1));	
}

TEST(HazardCurvePiecewiseConstant, MultiplyHazardRate10) {
	std::vector<double> times = { 0., 2., 3. };
	std::vector<double> rates = { 0.1, 0.05, 0.01 };
	HazardCurvePiecewiseConstant orig(TimeSeries<double, double>(times, rates));
	const double f = 0.4;
	const double t0 = 3.1;
	const double t1 = 3.5;
	auto c1 = orig.multiply_hazard_rate(t0, t1, f);
	for (size_t i = 0; i < 2; ++i) {
		ASSERT_EQ(rates[i], c1->instantaneous_hazard_rate(times[i])) << i;
	}
	ASSERT_NEAR(f * rates[2], c1->instantaneous_hazard_rate(t0), 1E-15);
	ASSERT_EQ(rates[2], c1->instantaneous_hazard_rate(t1));
}

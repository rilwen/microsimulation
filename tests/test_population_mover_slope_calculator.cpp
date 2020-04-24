#include <gtest/gtest.h>
#include "core/population_mover_slope_calculator.hpp"
#include "core/stl_utils.hpp"

using namespace averisera;

static void test_impl(const std::vector<double>& distr, const size_t from_idx, const std::vector<double>& expected_a, const std::vector<double>& expected_b) {
	PopulationMoverSlopeCalculator calc(1e-9);
	std::vector<double> a;
	std::vector<double> b;
	Eigen::VectorXd d(distr.size());
	std::copy(distr.begin(), distr.end(), d.data());
	ASSERT_EQ(distr.size(), d.size());
	calc.calculate(d, from_idx, a, b);
	ASSERT_EQ(distr.size(), a.size());
	ASSERT_EQ(distr.size(), b.size());
	for (size_t i = 0; i < distr.size(); ++i) {
		ASSERT_NEAR(expected_a[i], a[i], 1e-8) << "a[" << i << "]: " << " " << a[i] << " " << a;
		ASSERT_NEAR(expected_b[i], b[i], 1e-8) << i << " " << b[i] << " " << b;
	}
}

TEST(PopulationMoverSlopeCalculator, Test1) {
	test_impl({ 0, 1 }, 0, { 0, 1 }, { 0, 0 });
}

TEST(PopulationMoverSlopeCalculator, Test2) {
	test_impl({ 0, 1 }, 1, { 0, 1 }, { 0, 0 });
}

TEST(PopulationMoverSlopeCalculator, Test3) {
	test_impl({ 1, 0 }, 0, { 1, 0 }, { 0, 0 });
}

TEST(PopulationMoverSlopeCalculator, Test4) {
	test_impl({ 1, 0 }, 1, { 1, 0 }, { 0, 0 });
}

TEST(PopulationMoverSlopeCalculator, Test5) {
	test_impl({ 0.3, 0.4, 0.3 }, 0, { 0.3, 0.4, 0.3 }, { 0, 0, 0 });
}

TEST(PopulationMoverSlopeCalculator, Test6) {
	test_impl({ 0.3, 0.4, 0.3 }, 1, { 0.6, 0.4, 0.0 }, { -0.6, 0, 0.6 });
}

TEST(PopulationMoverSlopeCalculator, Test7) {
	test_impl({ 0.3, 0.4, 0.3 }, 2, { 0.3, 0.4, 0.3 }, { 0, 0, 0 });
}

TEST(PopulationMoverSlopeCalculator, Test8) {
	test_impl({ 0.26, 0.11, 0.24, 0.27, 0.12 }, 2, { 0.52, 0.22, 0.24, 0.0, 0.02 }, { -0.52, -0.22, 0, 0.54, 0.2 });
}

TEST(PopulationMoverSlopeCalculator, Test9) {
	test_impl({ 0.5, 0.5 }, 0, { 0.5, 0.5 }, { 0, 0 });
}

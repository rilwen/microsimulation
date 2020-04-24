#include <gtest/gtest.h>
#include "core/filter_iterator.hpp"
#include <cmath>
#include <vector>

using namespace averisera;

TEST(FilterIterator, Vector) {
	std::vector<double> input = { std::numeric_limits<double>::infinity() , -0.2, std::numeric_limits<double>::quiet_NaN(), 0.3, std::numeric_limits<double>::infinity(), 10,  -std::numeric_limits<double>::infinity() };
	std::vector<double> output(3);
	const auto pred = [](const double& x) { return std::isfinite(x); };
	std::copy(make_filter_iterator(input.begin(), input.end(), pred), make_filter_iterator(input.end(), input.end(), pred), output.begin());
	ASSERT_EQ(std::vector<double>({ -0.2, 0.3, 10 }), output);
}
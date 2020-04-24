#include <gtest/gtest.h>
#include "core/kruger_approximation.hpp"

using namespace averisera;

TEST(KrugerApproximation, AllPositive) {
	const std::vector<double> x({ 0.0, 1.0, 2.2, 3.0, 4.0 });
	const std::vector<double> y({ 0.0, 0.07, 0.22, 0.24, 0.41 });
	std::vector<double> dy(x.size());
	KrugerApproximation<double>::calculate(x, y, dy);
	for (size_t i = 0; i < x.size(); ++i) {
		ASSERT_GT(dy[i], 0.0) << i;
	}
}

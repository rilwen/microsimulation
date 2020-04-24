#include <gtest/gtest.h>
#include "core/kahan_summation.hpp"
#include <cmath>

TEST(KahanSummation, Constructor) {
	averisera::KahanSummation<double> x(2.1);
	ASSERT_EQ(2.1, x);
}

TEST(KahanSummation, Getters) {
	averisera::KahanSummation<double> x;
	double converted = x;
	ASSERT_EQ(0, converted);
	x = 2.1;
	converted = x;
	ASSERT_EQ(2.1, converted);
}

TEST(KahanSummation, Assign) {
	averisera::KahanSummation<double> x;
	x = 2.1;
	ASSERT_EQ(2.1, x);
}

TEST(KahanSummation, Accuracy) {
	averisera::KahanSummation<double> sum(1.0);
	for (int i = 0; i < 10; ++i) {
		sum += 1E-16; // adding 1E-16 to 1 will give 1 in simple summation, but Kahan summation will be more accurate
	}
	ASSERT_NEAR(1 + 1E-15, sum, 1E-17);
}

TEST(KahanSummation, Infinities) {
	averisera::KahanSummation<double> sum(1.0);
	sum += std::numeric_limits<double>::infinity();
	sum += 2.0;
	ASSERT_EQ(std::numeric_limits<double>::infinity(), sum);
	sum = 0;
	sum += (-std::numeric_limits<double>::infinity());
	sum += 2.0;
	ASSERT_EQ(-std::numeric_limits<double>::infinity(), sum);
	sum += std::numeric_limits<double>::infinity();
	ASSERT_EQ(FP_NAN, std::fpclassify(sum));
	sum = 0.4;
	sum += std::numeric_limits<double>::quiet_NaN();
	ASSERT_EQ(FP_NAN, std::fpclassify(sum));
}

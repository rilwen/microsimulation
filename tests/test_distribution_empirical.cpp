#include <gtest/gtest.h>
#include "core/distribution_empirical.hpp"

using namespace averisera;

TEST(DistributionEmpirical, Test) {
	// -0.1, 0.1, 0.1, 0.2, 0.5, 0.5
	const std::vector<double> sample({ 0.1, 0.5, 0.1, -0.1, 0.2, 0.5 });
	DistributionEmpirical distr(sample);
	ASSERT_EQ(-0.1, distr.infimum());
	ASSERT_EQ(0.5, distr.supremum());
	ASSERT_TRUE(std::isinf(distr.pdf(0.1)));
	ASSERT_EQ(0., distr.pdf(0));
	ASSERT_NEAR(1. / 6., distr.cdf(-0.1), 1E-15);
	ASSERT_NEAR(1. / 6., distr.cdf(-0.05), 1E-15);
	ASSERT_NEAR(0.5, distr.cdf(0.1), 1E-15);
	ASSERT_NEAR(0.5, distr.cdf(0.19), 1E-15);
	ASSERT_NEAR(4. / 6., distr.cdf(0.2), 1E-15);
	ASSERT_NEAR(4. / 6., distr.cdf(0.21), 1E-15);
	ASSERT_EQ(1, distr.cdf(0.5));
	ASSERT_EQ(0, distr.cdf(-0.11));
	ASSERT_EQ(1, distr.cdf(0.5));
	ASSERT_EQ(-0.1, distr.icdf(0));
	ASSERT_EQ(0.5, distr.icdf(1));
	EXPECT_EQ(0.1, distr.icdf(0.49));
	EXPECT_EQ(0.1, distr.icdf(0.5));
	EXPECT_EQ(0.2, distr.icdf(0.51));
	for (double x : sample) {
		EXPECT_EQ(x, distr.icdf(distr.cdf(x))) << x;
	}

	ASSERT_NEAR(0., distr.cdf2(-0.1), 1E-15);
	ASSERT_NEAR(1./6., distr.cdf2(0.1), 1E-15);
	ASSERT_NEAR(0.5, distr.cdf2(0.19), 1E-15);
	ASSERT_NEAR(0.5, distr.cdf2(0.2), 1E-15);
	ASSERT_NEAR(4./6., distr.cdf2(0.21), 1E-15);
	ASSERT_EQ(4./6., distr.cdf2(0.5));
	ASSERT_EQ(0, distr.cdf2(-0.11));
	ASSERT_EQ(1, distr.cdf2(0.500001));
}

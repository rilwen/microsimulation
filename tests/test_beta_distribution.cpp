#include <gtest/gtest.h>
#include "core/beta_distribution.hpp"

using namespace averisera;

TEST(BetaDistribution, EstimateFromSample) {
	std::vector<double> sample({ 0.0, 0.5, 0.5, 1.0, std::numeric_limits<double>::quiet_NaN() });
	BetaDistribution distr(BetaDistribution::estimate(sample.begin(), sample.end()));
	ASSERT_NEAR(0.5, distr.mean(), 1e-15);
	ASSERT_NEAR(distr.alpha(), distr.beta(), 1e-15);
	ASSERT_EQ(0.0, distr.infimum());
	ASSERT_EQ(1.0, distr.supremum());
	ASSERT_NEAR(0.5, distr.icdf(0.5), 1e-15);
	RunningStatistics<double> rs;
	for (double x : sample) {
		rs.add_if_not_nan(x);
	}
	ASSERT_NEAR(rs.variance(), distr.variance(distr.mean()), 1e-15);
	sample.push_back(std::numeric_limits<double>::infinity());
	ASSERT_THROW(BetaDistribution::estimate(sample.begin(), sample.end()), std::runtime_error);
	ASSERT_THROW(BetaDistribution::estimate_given_range(sample.begin(), sample.end(), 0.1, 0.9), std::runtime_error);
}

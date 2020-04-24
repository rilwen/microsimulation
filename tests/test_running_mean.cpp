#include <gtest/gtest.h>
#include "core/running_mean.hpp"
#include <random>
#include <cmath>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>
#include "testing/assertions.hpp"

TEST(RunningMean, Count) {
	averisera::RunningMean<double> rs;
	ASSERT_EQ(0u, rs.nbr_samples());
	rs.add(0);
	ASSERT_EQ(1u, rs.nbr_samples());
	rs.add(1);
	ASSERT_EQ(2u, rs.nbr_samples());
}

TEST(RunningMean, AddIfFinite) {
	averisera::RunningMean<double> rs;
	rs.add_if_finite(1);
	ASSERT_EQ(1u, rs.nbr_samples());
	rs.add_if_finite(std::numeric_limits<double>::infinity());
	rs.add_if_finite(-std::numeric_limits<double>::infinity());
	rs.add_if_finite(std::numeric_limits<double>::quiet_NaN());
	ASSERT_EQ(1u, rs.nbr_samples());
	rs.add_if_finite(std::vector<double>({ 0.5 }));
	ASSERT_EQ(2u, rs.nbr_samples());
	rs.add_if_finite(std::vector<double>({ 0.5, std::numeric_limits<double>::infinity() }));
	rs.add_if_finite(std::vector<double>({ 0.5, -std::numeric_limits<double>::infinity() }));
	rs.add_if_finite(std::vector<double>({ 0.5, std::numeric_limits<double>::quiet_NaN() }));
	ASSERT_EQ(2u, rs.nbr_samples());
}

TEST(RunningMean, AddFinite) {
	averisera::RunningMean<double> rs;
	rs.add_finite(1.0);
	ASSERT_EQ(1.0, rs.mean());
	ASSERT_EQ(1u, rs.nbr_samples());
    rs.add_finite(2.0);
	ASSERT_NEAR(1.5, rs.mean(), 1e-12);
	ASSERT_EQ(2u, rs.nbr_samples());
}

TEST(RunningMean, AddIfNotNaN) {
	averisera::RunningMean<double> rs;
	rs.add_if_not_nan(1);
	ASSERT_EQ(1u, rs.nbr_samples());
	ASSERT_EQ(1.0, rs.mean());
	rs.add_if_not_nan(std::numeric_limits<double>::quiet_NaN());
	ASSERT_EQ(1u, rs.nbr_samples());
	rs.add_if_not_nan(std::numeric_limits<double>::infinity());
	ASSERT_EQ(2u, rs.nbr_samples());
	ASSERT_EQ(std::numeric_limits<double>::infinity(), rs.mean());
	rs.add_if_not_nan(std::numeric_limits<double>::infinity());
	ASSERT_EQ(3u, rs.nbr_samples());
	ASSERT_EQ(std::numeric_limits<double>::infinity(), rs.mean());
}

TEST(RunningMean, Mean) {
	averisera::RunningMean<double> rs;
	ASSERT_TRUE(std::isnan(rs.mean()));
	rs.add(0);
	ASSERT_NEAR(0., rs.mean(), 1E-15);
	rs.add(-1);
	ASSERT_NEAR(-0.5, rs.mean(), 1E-15);
	rs.add(1);
	ASSERT_NEAR(0., rs.mean(), 1E-15);
}

TEST(RunningMean, FromVector) {
	averisera::RunningMean<double> rs;
	rs.add(std::vector<double>({ 0.5, 0.5 }));
	ASSERT_NEAR(1, rs.mean(), 1E-15);
}


TEST(RunningMean, SmallSample) {
	averisera::RunningMean<double> rs;
	rs.add(1.2);
	rs.add(-0.5);
	rs.add(2.5);
	rs.add(-1.7);
	ASSERT_NEAR(0.375, rs.mean(), 1E-17);
}

TEST(RunningMean, LargeSample) {
	const unsigned int npath = 200000;
	std::mt19937 rng(37);
	const double mu = 204.12;
	const double sigma = 51;
	boost::normal_distribution<double> nd(mu, sigma);
	boost::variate_generator<std::mt19937&, boost::normal_distribution<double>> gen(rng, nd);
	averisera::RunningMean<double> rs;
	for (unsigned int i = 0; i < npath; ++i) {
		rs.add(gen());
	}
	ASSERT_NEAR(mu, rs.mean(), 1e-2 * sigma);
}

TEST(RunningMean, InfiniteSamplePositive) {
	averisera::RunningMean<double> rs;
	rs.add(std::numeric_limits<double>::infinity());
	ASSERT_EQ(std::numeric_limits<double>::infinity(), rs.mean());
	rs.add(std::numeric_limits<double>::infinity());
	ASSERT_EQ(std::numeric_limits<double>::infinity(), rs.mean());
}

TEST(RunningMean, InfiniteSampleNegative) {
	averisera::RunningMean<double> rs;
	rs.add(-std::numeric_limits<double>::infinity());
	ASSERT_EQ(-std::numeric_limits<double>::infinity(), rs.mean());
	rs.add(-std::numeric_limits<double>::infinity());
	ASSERT_EQ(-std::numeric_limits<double>::infinity(), rs.mean());
}

TEST(RunningMean, InfiniteSampleSwitchSign) {
	averisera::RunningMean<double> rs;
	rs.add(std::numeric_limits<double>::infinity());
	ASSERT_EQ(std::numeric_limits<double>::infinity(), rs.mean());
	rs.add(-std::numeric_limits<double>::infinity());
	ASSERT_TRUE(std::isnan(rs.mean()));
}

TEST(RunningMean, NaNSamples) {
	averisera::RunningMean<double> rs;
	rs.add(std::numeric_limits<double>::quiet_NaN());
	ASSERT_TRUE(std::isnan(rs.mean()));
	rs.add(std::numeric_limits<double>::quiet_NaN());
	ASSERT_TRUE(std::isnan(rs.mean()));
}

#include <gtest/gtest.h>
#include "core/running_statistics.hpp"
#include "core/sacado_scalar.hpp"
#include <random>
#include <cmath>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>
#include "testing/assertions.hpp"

using namespace averisera;

TEST(RunningStatistics,Count) {
	RunningStatistics<double> rs;
	ASSERT_EQ(0u, rs.nbr_samples());
	rs.add(0);
	ASSERT_EQ(1u, rs.nbr_samples());
	rs.add(1);
	ASSERT_EQ(2u, rs.nbr_samples());
}

TEST(RunningStatistics, AddIfFinite) {
	RunningStatistics<double> rs;
	rs.add_if_finite(1);
	ASSERT_EQ(1u, rs.nbr_samples());
	rs.add_if_finite(std::numeric_limits<double>::infinity());
	rs.add_if_finite(-std::numeric_limits<double>::infinity());
	rs.add_if_finite(std::numeric_limits<double>::quiet_NaN());
	ASSERT_EQ(1u, rs.nbr_samples());
	rs.add_if_finite(std::vector<double>({0.5}));
	ASSERT_EQ(2u, rs.nbr_samples());
	rs.add_if_finite(std::vector<double>({ 0.5, std::numeric_limits<double>::infinity() }));
	rs.add_if_finite(std::vector<double>({ 0.5, -std::numeric_limits<double>::infinity() }));
	rs.add_if_finite(std::vector<double>({ 0.5, std::numeric_limits<double>::quiet_NaN() }));
	ASSERT_EQ(2u, rs.nbr_samples());
}

TEST(RunningStatistics, AddIfNotNaN) {
	RunningStatistics<double> rs;
	rs.add_if_not_nan(1);
	ASSERT_EQ(1u, rs.nbr_samples());
	ASSERT_EQ(1.0, rs.mean());
	rs.add_if_not_nan(std::numeric_limits<double>::quiet_NaN());
	ASSERT_EQ(1u, rs.nbr_samples());
	rs.add_if_not_nan(std::numeric_limits<double>::infinity());
	ASSERT_EQ(2u, rs.nbr_samples());
	ASSERT_EQ(std::numeric_limits<double>::infinity(), rs.mean());
	ASSERT_TRUE(std::isnan(rs.variance()));
	ASSERT_TRUE(std::isnan(rs.skewness()));
	ASSERT_TRUE(std::isnan(rs.kurtosis()));
	rs.add_if_not_nan(std::numeric_limits<double>::infinity());
	ASSERT_EQ(3u, rs.nbr_samples());
	ASSERT_EQ(std::numeric_limits<double>::infinity(), rs.mean());
}

TEST(RunningStatistics,Mean) {
	RunningStatistics<double> rs;
	rs.add(0);
    ASSERT_NEAR(0., rs.mean(), 1E-15);
	rs.add(-1);
    ASSERT_NEAR(-0.5, rs.mean(), 1E-15);
	rs.add(1);
	ASSERT_NEAR(0., rs.mean(), 1E-15);
}

TEST(RunningStatistics, FromVector) {
	RunningStatistics<double> rs;
	rs.add(std::vector<double>({ 0.5, 0.5 }));
	ASSERT_NEAR(1, rs.mean(), 1E-15);
}

TEST(RunningStatistics,Min) {
	RunningStatistics<double> rs;
	rs.add(0);
	rs.add(-1);
	rs.add(1);
	ASSERT_EQ(-1, rs.min());
}

TEST(RunningStatistics,Max) {
	RunningStatistics<double> rs;
	rs.add(0);
	rs.add(-1);
	rs.add(1);
	ASSERT_EQ(1, rs.max());
}

TEST(RunningStatistics,Variance) {
	RunningStatistics<double> rs;
	rs.add(1);
	rs.add(0);
	rs.add(2);
	ASSERT_NEAR(1, rs.variance(), 1E-15);
}

TEST(RunningStatistics,StandardDeviation) {
	RunningStatistics<long double> rs;
	rs.add(0);
	rs.add(-1);
	rs.add(0.5);
	rs.add(1);
	rs.add(0);
    const long double var = rs.variance();
    const long double std_from_var = std::sqrt(var);
    const long double std = rs.standard_deviation();
    ASSERT_TRUE(averisera::testing::is_near(std, std_from_var, 1E-18L)); // ASSERT_NEAR doesn't work with long double
}

TEST(RunningStatistics,Kurtosis) {
	RunningStatistics<double> rs;
	const size_t n = 10000;
	for (size_t i = 0; i < n; ++i) {
		rs.add( (static_cast<double>(i) + 0.5) / static_cast<double>(n) );
	}
	ASSERT_NEAR(-1.2, rs.kurtosis(), 1E-7);
}

TEST(RunningStatistics,Skewness) {
	RunningStatistics<double> rs;
	const size_t n = 100000;
	for (size_t i = 0; i < n; ++i) {
		const double uniform = (static_cast<double>(i) + 0.5) / static_cast<double>(n);
		const double exponential = -log(1 - uniform);
		rs.add( exponential );
	}
	ASSERT_NEAR(2, rs.skewness(), 2E-3);
}

TEST(RunningStatistics, SmallSample) {
	RunningStatistics<double> rs;
	rs.add(1.2);
	rs.add(-0.5);
	rs.add(2.5);
	rs.add(-1.7);
	ASSERT_EQ(-1.7, rs.min());
	ASSERT_EQ(2.5, rs.max());
	ASSERT_NEAR(0.375, rs.mean(), 1E-17);
	ASSERT_NEAR(3.4225, rs.variance(), 5E-16);
	ASSERT_NEAR(1.85, rs.standard_deviation(), 4E-16);
	ASSERT_NEAR(0.033624529526570766, rs.skewness(), 4E-16); // compare to scipy.stats.skew(x, bias=True)
	ASSERT_NEAR(-1.4830917229985403, rs.kurtosis(), 1E-15); // scipy.stats.kurtosis(x, bias=True, fisher=True)
}

TEST(RunningStatistics, LargeSymmetricSample) {
	const unsigned int npath = 200000;
	std::mt19937 rng(37);
	boost::normal_distribution<double> nd(0, 51);
	boost::variate_generator<std::mt19937&, boost::normal_distribution<double>> gen(rng, nd);
	RunningStatistics<double> rs;
	for (unsigned int i = 0; i < npath; ++i) {
		const int x = static_cast<int>(std::round(gen()));
		rs.add(x);
		rs.add(-x);
	}
	ASSERT_NEAR(0., rs.mean(), 2E-17);
	ASSERT_NEAR(51 * 51 * npath / (npath - 1.0), rs.variance(), 4);
	ASSERT_NEAR(51 * sqrt(npath / (npath - 1.0)), rs.standard_deviation(), 0.04);
	ASSERT_NEAR(0., rs.skewness(), 2E-17);
	ASSERT_NEAR(0., rs.kurtosis(), 0.05);
}

TEST(RunningStatistics, InfiniteSamplePositive) {
	RunningStatistics<double> rs;
	rs.add(std::numeric_limits<double>::infinity());
	ASSERT_EQ(std::numeric_limits<double>::infinity(), rs.mean());
	ASSERT_TRUE(std::isnan(rs.variance()));
	ASSERT_TRUE(std::isnan(rs.skewness()));
	ASSERT_TRUE(std::isnan(rs.kurtosis()));
	rs.add(std::numeric_limits<double>::infinity());
	ASSERT_EQ(std::numeric_limits<double>::infinity(), rs.mean());
	ASSERT_TRUE(std::isnan(rs.variance()));
	ASSERT_TRUE(std::isnan(rs.skewness()));
	ASSERT_TRUE(std::isnan(rs.kurtosis()));
}

TEST(RunningStatistics, InfiniteSampleNegative) {
	RunningStatistics<double> rs;
	rs.add(-std::numeric_limits<double>::infinity());
	ASSERT_EQ(-std::numeric_limits<double>::infinity(), rs.mean());
	ASSERT_TRUE(std::isnan(rs.variance()));
	ASSERT_TRUE(std::isnan(rs.skewness()));
	ASSERT_TRUE(std::isnan(rs.kurtosis()));
	rs.add(-std::numeric_limits<double>::infinity());
	ASSERT_EQ(-std::numeric_limits<double>::infinity(), rs.mean());
	ASSERT_TRUE(std::isnan(rs.variance()));
	ASSERT_TRUE(std::isnan(rs.skewness()));
	ASSERT_TRUE(std::isnan(rs.kurtosis()));
}

TEST(RunningStatistics, InfiniteSampleSwitchSign) {
	RunningStatistics<double> rs;
	rs.add(-std::numeric_limits<double>::infinity());
	ASSERT_EQ(-std::numeric_limits<double>::infinity(), rs.mean());
	ASSERT_TRUE(std::isnan(rs.variance()));
	ASSERT_TRUE(std::isnan(rs.skewness()));
	ASSERT_TRUE(std::isnan(rs.kurtosis()));
	rs.add(std::numeric_limits<double>::infinity());
	ASSERT_TRUE(std::isnan(rs.mean()));
	ASSERT_TRUE(std::isnan(rs.variance()));
	ASSERT_TRUE(std::isnan(rs.skewness()));
	ASSERT_TRUE(std::isnan(rs.kurtosis()));
}

TEST(RunningStatistics, NaNSamples) {
	RunningStatistics<double> rs;
	rs.add(std::numeric_limits<double>::quiet_NaN());
	ASSERT_TRUE(std::isnan(rs.mean()));
	ASSERT_TRUE(std::isnan(rs.variance()));
	ASSERT_TRUE(std::isnan(rs.skewness()));
	ASSERT_TRUE(std::isnan(rs.kurtosis()));
	rs.add(std::numeric_limits<double>::quiet_NaN());
	ASSERT_TRUE(std::isnan(rs.mean()));
	ASSERT_TRUE(std::isnan(rs.variance()));
	ASSERT_TRUE(std::isnan(rs.skewness()));
	ASSERT_TRUE(std::isnan(rs.kurtosis()));
}

TEST(RunningStatistics, AutoDiff) {
    RunningStatistics<adouble> rs;
    RunningStatistics<double> rs2;
    unsigned int idx = 0;
    adouble x = double2adouble(1, idx, 1.0);
    ASSERT_EQ(1u, idx);
    rs.add(0 - x);
    rs2.add(0 - x.val());
    EXPECT_EQ(1u, rs.nbr_samples());
    ASSERT_NEAR(-1.0, rs.mean().val(), 5E-16);
    rs.add_if_finite(-1 - x);
    rs2.add(-1-x.val());
    rs.add_if_not_nan(1 - x);
    rs2.add(1 - x.val());
    ASSERT_EQ(3u, rs.nbr_samples());
    ASSERT_NEAR(-1.0, rs.mean().val(), 5E-16);
    ASSERT_NEAR(-1.0, rs.mean().dx(0), 5E-16);
    ASSERT_NEAR(1.0, rs.variance().val(), 5E-16);
    ASSERT_NEAR(0.0, rs.variance().dx(0), 1E-16);
    ASSERT_NEAR(1.0, rs.standard_deviation().val(), 5E-16);
    ASSERT_NEAR(0.0, rs.standard_deviation().dx(0), 1E-16);
    ASSERT_NEAR(rs2.skewness(), rs.skewness().val(), 1E-15);
    ASSERT_NEAR(0.0, rs.skewness().dx(0), 1E-16);
    ASSERT_NEAR(rs2.kurtosis(), rs.kurtosis().val(), 1E-15);
    ASSERT_NEAR(0.0, rs.kurtosis().dx(0), 1E-16);
}


TEST(RunningStatistics, Multiply) {
    const float x = -0.34f;
    const std::vector<double> sample({-0.3, 0.12, 1.4, -0.31, 2.0, 2.01, 0.34});
    RunningStatistics<double> rs1;
    RunningStatistics<double> rs2;
    for (double y: sample) {
        rs1.add(y);
        rs2.add(x * y);
    }
    RunningStatistics<double> rs3(rs1 * x);
    ASSERT_NEAR(rs2.mean(), rs3.mean(), 1E-14);
    ASSERT_EQ(rs2.min(), rs3.min());
    ASSERT_EQ(rs2.max(), rs3.max());
    ASSERT_NEAR(rs2.variance(), rs3.variance(), 1E-8);
    ASSERT_NEAR(rs2.standard_deviation(), rs3.standard_deviation(), 1E-8);
    ASSERT_NEAR(rs2.skewness(), rs3.skewness(), 2E-8);
    ASSERT_NEAR(rs2.kurtosis(), rs3.kurtosis(), 1E-7);
    ASSERT_EQ(rs2.nbr_samples(), rs3.nbr_samples());
}

TEST(RunningStatistics, Add) {
	const float x = -0.34f;
	const std::vector<double> sample({ -0.3, 0.12, 1.4, -0.31, 2.0, 2.01, 0.34 });
	RunningStatistics<double> rs1;
	RunningStatistics<double> rs2;
	for (double y : sample) {
		rs1.add(y);
		rs2.add(x + y);
	}
	RunningStatistics<double> rs3(rs1 + x);
	ASSERT_NEAR(rs2.mean(), rs3.mean(), 1E-14);
	ASSERT_EQ(rs2.min(), rs3.min());
	ASSERT_EQ(rs2.max(), rs3.max());
	ASSERT_NEAR(rs2.variance(), rs3.variance(), 1E-14);
	ASSERT_NEAR(rs2.standard_deviation(), rs3.standard_deviation(), 1E-14);
	ASSERT_NEAR(rs2.skewness(), rs3.skewness(), 1E-14);
	ASSERT_NEAR(rs2.kurtosis(), rs3.kurtosis(), 1E-14);
	ASSERT_EQ(rs2.nbr_samples(), rs3.nbr_samples());
}
    

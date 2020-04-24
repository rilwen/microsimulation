#include <gtest/gtest.h>
#include "core/distribution_linear_interpolated.hpp"

using namespace averisera;

TEST(DistributionLinearInterpolated, Constructor) {
    ASSERT_THROW(DistributionLinearInterpolated(std::vector<double>({0, 1}), std::vector<double>({0.2, 0.8})), std::domain_error);
    ASSERT_THROW(DistributionLinearInterpolated(std::vector<double>({0, 1, 2}), std::vector<double>({0.2, 0.3})), std::domain_error);
    ASSERT_THROW(DistributionLinearInterpolated(std::vector<double>({0, 1, 2}), std::vector<double>({-0.3, 1.3})), std::domain_error);
}

TEST(DistributionLinearInterpolated, PDF) {
    DistributionLinearInterpolated distr(std::vector<double>({0, 1, 3}), std::vector<double>({0.2, 0.8}));
	ASSERT_EQ(2u, distr.nbr_ranges());
    ASSERT_EQ(0., distr.pdf(-0.2));
    ASSERT_EQ(0., distr.pdf(3.1));
    ASSERT_NEAR(0.2, distr.pdf(0.5), 1E-15);
    ASSERT_NEAR(0.2, distr.pdf(0.), 1E-15);
    ASSERT_NEAR(0.2, distr.pdf(1.), 1E-15);
    ASSERT_NEAR(0.4, distr.pdf(2.), 1E-15);
    ASSERT_NEAR(0.4, distr.pdf(3.), 1E-15);
}

TEST(DistributionLinearInterpolated, CDF) {
    DistributionLinearInterpolated distr(std::vector<double>({ 0, 1, 3 }), std::vector<double>({ 0.2, 0.8 }));
    ASSERT_EQ(0., distr.cdf(-0.2));
    ASSERT_EQ(1., distr.cdf(3.1));
    ASSERT_NEAR(0.1, distr.cdf(0.5), 1E-15);
    ASSERT_NEAR(0., distr.cdf(0.), 1E-15);
    ASSERT_NEAR(0.2, distr.cdf(1.), 1E-15);
    ASSERT_NEAR(0.6, distr.cdf(2.), 1E-15);
    ASSERT_NEAR(1.0, distr.cdf(3.), 1E-15);
}

TEST(DistributionLinearInterpolated, ICDF) {
    DistributionLinearInterpolated distr(std::vector<double>({ 0, 1, 3 }), std::vector<double>({ 0.2, 0.8 }));
    ASSERT_EQ(0., distr.icdf(0.));
    ASSERT_EQ(3., distr.icdf(1));
    ASSERT_NEAR(0.5, distr.icdf(0.1), 1E-15);
    ASSERT_NEAR(0., distr.icdf(0.), 1E-15);
    ASSERT_NEAR(1., distr.icdf(0.2), 1E-15);
    ASSERT_NEAR(2., distr.icdf(0.6), 1E-15);
}

TEST(DistributionLinearInterpolated, Mean) {
    DistributionLinearInterpolated distr(std::vector<double>({ 0, 1, 3 }), std::vector<double>({ 0.2, 0.8 }));
    ASSERT_NEAR(0.2 * 0.5 + 0.8 * 2, distr.mean(), 1E-15);
}

TEST(DistributionLinearInterpolated, Variance) {
    DistributionLinearInterpolated distr(std::vector<double>({-1, 0, 1}), std::vector<double>({0.5, 0.5}));
    ASSERT_NEAR(0, distr.mean(), 1E-15);
    ASSERT_NEAR(1/3.0, distr.variance(0), 1E-14);
    distr = DistributionLinearInterpolated(std::vector<double>({0, 1}), std::vector<double>({1.0}));
    ASSERT_NEAR(1/12.0, distr.variance(distr.mean()), 1E-14);
}

TEST(DistributionLinearInterpolated, Estimate) {
	const std::vector<double> sample({ -0.5, 0.5, 1.5, 2.5, std::numeric_limits<double>::quiet_NaN() });
	const std::vector<double> boundaries({ -1.0, 0.0, 1.0, 2.0 });
	DistributionLinearInterpolated distr(DistributionLinearInterpolated::estimate(sample.begin(), sample.end(), std::vector<double>(boundaries), false, false));
	ASSERT_EQ(3u, distr.nbr_ranges());
	ASSERT_EQ(DistributionLinearInterpolated(boundaries, { 1.0 / 3, 1.0 / 3, 1.0 / 3 }), distr);
	ASSERT_THROW(DistributionLinearInterpolated::estimate(sample.begin(), sample.end(), std::vector<double>(boundaries), true, false), std::runtime_error);
	ASSERT_THROW(DistributionLinearInterpolated::estimate(sample.begin(), sample.end(), std::vector<double>(boundaries), false, true), std::runtime_error);
}
#include <gtest/gtest.h>
#include "core/distribution_shifted_lognormal.cpp"
#include "core/adapt_1d.hpp"
#include "core/rng_impl.hpp"
#include <limits>

using namespace averisera;

TEST(DistributionShiftedLognormal, Mean) {
    const double tol = 1E-6;
    Adapt1D_15 integrator(100, tol, true);
    DistributionShiftedLognormal distr(0.1, 0.7, -0.1);
	ASSERT_EQ(0.1, distr.log_mean());
	ASSERT_EQ(0.7, distr.log_sigma());
	ASSERT_EQ(-0.1, distr.shift());
    const double expect_mean = integrator.integrate([&distr](double p){ return distr.icdf(p); }, 1E-12, 1.0 - 1E-12);
    const double actual_mean = distr.mean();
    ASSERT_NEAR(expect_mean, actual_mean, 50*tol); 
}

TEST(DistributionShiftedLognormal, Variance) {
    Adapt1D_15 integrator(100, 1E-6, true);
    DistributionShiftedLognormal distr(0.1, 0.7, -0.1);
    const double actual_mean = distr.mean();
    const double expect_var = integrator.integrate([&distr, actual_mean](double p){
            const double w = distr.icdf(p) - actual_mean;
            return w * w;
        }, 1E-14, 1.0 - 1E-14);
    const double actual_var = distr.variance(actual_mean);
    ASSERT_NEAR(expect_var, actual_var, 5E-5); 
}

TEST(DistributionShiftedLognormal, CDF) {
    DistributionShiftedLognormal distr(0.1, 0.7, -0.1);
    ASSERT_NEAR(0.5, distr.cdf(exp(0.1) - 0.1), 1E-14);
    ASSERT_NEAR(0.0, distr.cdf(-0.1), 1E-14);
    ASSERT_EQ(1.0, distr.cdf(std::numeric_limits<double>::infinity()));
}

TEST(DistributionShiftedLognormal, ICDF) {
    DistributionShiftedLognormal distr(0.1, 0.7, -0.1);
    ASSERT_NEAR(exp(0.1) - 0.1, distr.icdf(0.5), 1E-14);
    ASSERT_NEAR(-0.1, distr.icdf(0), 1E-14);
    ASSERT_EQ(std::numeric_limits<double>::infinity(), distr.icdf(1));
}

TEST(DistributionShiftedLognormal, EstimateNoShift) {
    const double a = 0.0;
	const std::vector<double> sample({ a + exp(-1), a, a + exp(1), std::numeric_limits<double>::quiet_NaN() });
	DistributionShiftedLognormal distr(DistributionShiftedLognormal::estimate_given_shift(sample.begin(), sample.end(), a));
	ASSERT_EQ(a, distr.shift());
	ASSERT_NEAR(0.0, distr.log_mean(), 1E-16);
	ASSERT_NEAR(1.0, distr.log_sigma(), 4e-16);
}

TEST(DistributionShiftedLognormal, EstimateGivenShift) {
    const double a = 1.0;
	const std::vector<double> sample({ a + exp(-1), a, a + exp(1), std::numeric_limits<double>::quiet_NaN() });
	DistributionShiftedLognormal distr(DistributionShiftedLognormal::estimate_given_shift(sample.begin(), sample.end(), a));
	ASSERT_EQ(a, distr.shift());
	ASSERT_NEAR(0.0, distr.log_mean(), 1E-16);
	ASSERT_NEAR(1.0, distr.log_sigma(), 4e-16);
}

TEST(DistributionShiftedLognormal, Estimate) {
	const size_t n = 100;
	std::vector<double> sample(n);
	const double shift = 1.0;
	const double mu = 0.5;
	const double sigma = 0.25;
	DistributionShiftedLognormal distr(mu, sigma, shift);
	RNGImpl rng(42);
	for (size_t i = 0; i < n; ++i) {
		sample[i] = distr.draw(rng);
	}
	const double true_median = distr.median();
	const double true_mean = distr.mean();
	const double true_variance = distr.variance(true_mean);
	DistributionShiftedLognormal estimated(DistributionShiftedLognormal::estimate(sample));
    DistributionShiftedLognormal est_shft(DistributionShiftedLognormal::estimate_given_shift(sample.begin(), sample.end(), estimated.shift()));
    EXPECT_NEAR(mu, estimated.log_mean(), 2);
    EXPECT_NEAR(sigma, estimated.log_sigma(), 0.2);
    EXPECT_NEAR(est_shft.log_mean(), estimated.log_mean(), 1e-18);
    EXPECT_NEAR(est_shft.log_sigma(), estimated.log_sigma(), 1e-18);
    EXPECT_NEAR(true_median, estimated.median(), 0.03);
    EXPECT_NEAR(true_mean, estimated.mean(), 0.03);
    EXPECT_NEAR(true_variance, estimated.variance(estimated.mean()), 0.1);
}

TEST(DistributionShiftedLognormal, FitExactlyGivenShift) {
	ASSERT_THROW(DistributionShiftedLognormal::fit_exactly_given_shift(0, 0.1, 0.5, 1, 2), std::domain_error);
	ASSERT_THROW(DistributionShiftedLognormal::fit_exactly_given_shift(0.1, 0, 0.5, 1, 2), std::domain_error);
	ASSERT_THROW(DistributionShiftedLognormal::fit_exactly_given_shift(0.5, 0.5, 0.5, 1, 2), std::domain_error);
	ASSERT_THROW(DistributionShiftedLognormal::fit_exactly_given_shift(0.4, 0.2, 0.5, -1, 2), std::domain_error);
	ASSERT_THROW(DistributionShiftedLognormal::fit_exactly_given_shift(0.4, 0.2, 0.5, 1, 1), std::domain_error);
	DistributionShiftedLognormal distr = DistributionShiftedLognormal::fit_exactly_given_shift(0.4, 0.25, 0.5, 1, 2);
	ASSERT_NEAR(0.4, distr.range_prob(0.5, 1), 1e-10);
	ASSERT_NEAR(0.25, distr.range_prob(1, 2), 1e-10);
	ASSERT_NEAR(0.35, distr.range_prob(2, std::numeric_limits<double>::infinity()), 1e-10);
}

TEST(DistributionShiftedLognormal, EstimateGivenShiftDiscrete) {
	std::vector<double> x({ 0.5, 1, 2 });
	std::vector<double> p({ 0.4, 0.25, 0.35 });
	DistributionShiftedLognormal distr = DistributionShiftedLognormal::estimate_given_shift(x, p);
	ASSERT_NEAR(0.4, distr.range_prob(0.5, 1), 1e-8);
	ASSERT_NEAR(0.25, distr.range_prob(1, 2), 1e-8);
	ASSERT_NEAR(0.35, distr.range_prob(2, std::numeric_limits<double>::infinity()), 1e-8);

	x = { 0.5, 1 };
	p = { 0.65, 0.35 };
	distr = DistributionShiftedLognormal::estimate_given_shift(x, p);
	ASSERT_NEAR(0.65, distr.range_prob(0.5, 1), 1e-8);
	ASSERT_NEAR(0.35, distr.range_prob(1, std::numeric_limits<double>::infinity()), 1e-8);
}

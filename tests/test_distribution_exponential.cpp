// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/distribution_exponential.hpp"
#include "core/adapt_1d.hpp"
#include <limits>

using namespace averisera;

static const double tol = 1E-12;
static Adapt1D_15 integrator(100, tol, true);

TEST(DistributionExponential, PDF) {
    DistributionExponential distr(0.5, 1.5);
    const double pdf_integral = integrator.integrate([&distr](double x){ return distr.pdf(x); }, -400, 400);
    ASSERT_NEAR(1.0, pdf_integral, tol);
}

TEST(DistributionExponential, CDF) {
    DistributionExponential distr_symmetric(1, 1);
    ASSERT_NEAR(0.5, distr_symmetric.cdf(0), 1E-14);
    ASSERT_NEAR(1.0, distr_symmetric.cdf(std::numeric_limits<double>::infinity()), 1E-14);
    ASSERT_NEAR(0.0, distr_symmetric.cdf(-std::numeric_limits<double>::infinity()), 1E-14);
    DistributionExponential distr(0.5, 1.5);
    const double expect_proba = integrator.integrate([&distr](double x){ return distr.pdf(x); }, -0.4, 0.6);
    const double actual_proba = distr.cdf(0.6) - distr.cdf(-0.4);
    ASSERT_NEAR(expect_proba, actual_proba, tol);
}

TEST(DistributionExponential, ICDF) {
    DistributionExponential distr(0.5, 1.5);
    double x = 0.6;
    double p = distr.cdf(x);
    ASSERT_NEAR(x, distr.icdf(p), 1E-14);
    x = -0.4;
    p = distr.cdf(x);
    ASSERT_NEAR(x, distr.icdf(p), 1E-14);
}

TEST(DistributionExponential, Mean) {
    DistributionExponential distr_symmetric(1, 1);
    ASSERT_NEAR(0.0, distr_symmetric.mean(), 1E-14);
    DistributionExponential distr(0.5, 1.5);
    const double expect_mean = integrator.integrate([&distr](double p){ return distr.icdf(p); }, tol, 1.0 - tol);
    const double actual_mean = distr.mean();
    ASSERT_NEAR(expect_mean, actual_mean, 50*tol);
}

TEST(DistributionExponential, Variance) {
    Adapt1D_15 integrator2(100, 1E-6, true);
    DistributionExponential distr(0.5, 1.5);
    const double actual_mean = distr.mean();
    const double expect_var = integrator2.integrate([&distr, actual_mean](double p){
            const double w = distr.icdf(p) - actual_mean;
            return w * w;
        }, 1E-14, 1.0 - 1E-14);
    const double actual_var = distr.variance(actual_mean);
    ASSERT_NEAR(expect_var, actual_var, 5E-5); 
}

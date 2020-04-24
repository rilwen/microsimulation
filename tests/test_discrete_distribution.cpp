/*
(C) Averisera Ltd 2014
*/
#include <gtest/gtest.h>
#include "core/discrete_distribution.hpp"
#include "core/rng_impl.hpp"
#include "core/running_mean.hpp"
#include <Eigen/Core>
#include <cmath>

using namespace averisera;

TEST(DiscreteDistribution,Test) {
	const int a = -1;
	const int b = 1;
	const size_t size = (b - a) + 1;
	std::vector<double> p(size);
	p[0] = 0.25;
	p[1] = 0.4;
	p[2] = 0.35;
	DiscreteDistribution dd(a, p);
	ASSERT_EQ(size, dd.size());
    ASSERT_EQ(1u, dd.dim());

    ASSERT_EQ(a, dd.lower_bound());
    ASSERT_EQ(b, dd.upper_bound());

	ASSERT_NEAR(0, dd.cdf(a-0.1), 1E-15);
	ASSERT_NEAR(0.25, dd.cdf(a), 1E-15);
    ASSERT_NEAR(0.25, dd.range_prob(-3, a), 1E-15);
    ASSERT_NEAR(0., dd.range_prob2(-3, a), 1E-15);
    ASSERT_NEAR(0., dd.cdf2(a), 1E-15);
	ASSERT_NEAR(0.25, dd.cdf(a + 0.1), 1E-15);
	ASSERT_NEAR(0.25, dd.cdf(a + 0.9), 1E-15);
    ASSERT_NEAR(0.25, dd.cdf2(a + 0.1), 1E-15);
	ASSERT_NEAR(0.25, dd.cdf2(a + 0.9), 1E-15);
    ASSERT_NEAR(0.25, dd.range_prob(-3, a + 0.1), 1E-15);
    ASSERT_NEAR(0.25, dd.range_prob2(-3.0, a + 0.1), 1E-15);
	ASSERT_NEAR(0.65, dd.cdf(a + 1), 1E-15);
    ASSERT_NEAR(0.25, dd.cdf2(a + 1), 1E-15);
	ASSERT_NEAR(0.65, dd.cdf(a + 1.1), 1E-15);
	ASSERT_NEAR(0.65, dd.cdf(a + 1.9), 1E-15);
    ASSERT_NEAR(0.65, dd.cdf2(a + 1.1), 1E-15);
	ASSERT_NEAR(0.65, dd.cdf2(a + 1.9), 1E-15);
	ASSERT_NEAR(1.0, dd.cdf(a + 2), 1E-15);
    ASSERT_NEAR(0.65, dd.cdf2(a + 2), 1E-15);
	ASSERT_NEAR(1.0, dd.cdf(a + 2.1), 1E-15);
    ASSERT_NEAR(1.0, dd.cdf2(a + 2.1), 1E-15);

    ASSERT_NEAR(0.75, dd.range_prob(a, b), 1E-15);
    ASSERT_NEAR(0.65, dd.range_prob2(a, b), 1E-15);

	for (size_t i = 0; i < size; ++i) {
		const double x = a + static_cast<int>(i); // enforce addition with sign
		ASSERT_EQ(0.0, dd.pdf(x - 0.1));
		ASSERT_EQ(std::numeric_limits<double>::infinity(), dd.pdf(x)) << x;
		ASSERT_EQ(0.0, dd.pdf(x + 0.1));
	}

	ASSERT_EQ(a, dd.icdf(0));
	ASSERT_EQ(a, dd.icdf(0.1));
	ASSERT_EQ(a, dd.icdf(p[0] - 0.01));
	ASSERT_EQ(a + 1, dd.icdf(p[0]));
	ASSERT_EQ(a + 1, dd.icdf(p[0] + 0.01));
	ASSERT_EQ(a + 1, dd.icdf(p[0] + p[1] - 0.01));
	ASSERT_EQ(b, dd.icdf(p[0] + p[1]));
	ASSERT_EQ(b, dd.icdf(p[0] + p[1] + 0.01));
	ASSERT_EQ(b, dd.icdf(0.99));
	ASSERT_EQ(b, dd.icdf(1));
	ASSERT_EQ(dd.icdf(0.52), dd.icdf_generic(0.52));

	ASSERT_EQ(p[0], dd.prob(a));
	ASSERT_EQ(p[1], dd.prob(a + 1));
	ASSERT_EQ(p[2], dd.prob(a + 2));
}

TEST(DiscreteDistribution, OneOfMany) {
	DiscreteDistribution dd(0, 2, 4);
	ASSERT_EQ(0, dd.lower_bound());
	ASSERT_EQ(4, dd.upper_bound());
	ASSERT_EQ(5u, dd.size());
	ASSERT_EQ(0.0, dd.prob(0));
	ASSERT_EQ(0.0, dd.prob(1));
	ASSERT_EQ(1.0, dd.prob(2));
	ASSERT_EQ(0.0, dd.prob(3));
	ASSERT_EQ(0.0, dd.prob(4));
	ASSERT_EQ(0.0, dd.cdf(0));
	ASSERT_EQ(0.0, dd.cdf(1));
	ASSERT_EQ(1.0, dd.cdf(2));
	ASSERT_EQ(1.0, dd.cdf(3));
	ASSERT_EQ(1.0, dd.cdf(4));
}

TEST(DiscreteDistribution, FromEigen) {
	Eigen::MatrixXd p(2, 2);
	p(0, 0) = 0.4;
	p(1, 0) = 0.6;
	DiscreteDistribution d(0, p.col(0));
	ASSERT_EQ(0, d.lower_bound());
	ASSERT_EQ(1, d.upper_bound());
	ASSERT_EQ(0.4, d.prob(0));
	ASSERT_EQ(0.6, d.prob(1));
}

TEST(DiscreteDistribution, MoveConstructors) {
    std::vector<double> p({0.4, 0.6});
    DiscreteDistribution d(1, std::move(p));
    ASSERT_EQ(0u, p.size());
    ASSERT_EQ(1, d.lower_bound());
	ASSERT_EQ(2, d.upper_bound());
    ASSERT_EQ(0.4, d.prob(1));
	ASSERT_EQ(0.6, d.prob(2));
    ASSERT_EQ(2, d.icdf(1.0));
    ASSERT_EQ(1, d.icdf(0.2));
    DiscreteDistribution d2(std::move(d));
    ASSERT_EQ(0u, d.size());
    ASSERT_EQ(1, d2.lower_bound());
	ASSERT_EQ(2, d2.upper_bound());
    ASSERT_EQ(0.4, d2.prob(1));
	ASSERT_EQ(0.6, d2.prob(2));
    ASSERT_EQ(2, d2.icdf(1.0));
    ASSERT_EQ(1, d2.icdf(0.2));
}

TEST(DiscreteDistribution, MeanVariance) {
    DiscreteDistribution d(-1, std::vector<double>({0.2, 0.3, 0.5}));
    const double mean = d.mean();
    ASSERT_NEAR(0.5 - 0.2, mean, 1E-15);
    const double variance = d.variance(mean);
    ASSERT_NEAR(pow(-1 - mean, 2) * 0.2 + pow(-mean, 2) * 0.3 + pow(1 - mean, 2) * 0.5, variance, 1E-15);
}

TEST(DiscreteDistribution, ConditionalMeanVariance) {
    DiscreteDistribution d(-1, std::vector<double>({0.2, 0.3, 0.5}));
    const double expected_mean = 0.5 - 0.2;
    const double expected_variance = pow(-1 - expected_mean, 2) * 0.2 + pow(-expected_mean, 2) * 0.3 + pow(1 - expected_mean, 2) * 0.5;
    ASSERT_NEAR(expected_mean, d.mean(), 1E-15);
    ASSERT_NEAR(expected_variance, d.variance(expected_mean), 1E-15);
    ASSERT_EQ(d.mean(), d.conditional_mean(-2, 2));
    ASSERT_EQ(d.variance(expected_mean), d.conditional_variance(expected_mean, -2, 2));
    const double mu11 = (-1 * 0.2) / 0.5;
    ASSERT_NEAR(mu11, d.conditional_mean(-1, 1), 1E-15);
    ASSERT_NEAR(0, d.conditional_mean(0, 1), 1E-15);
    ASSERT_NEAR(-1, d.conditional_mean(-1.1, -0.5), 1E-15);
    ASSERT_NEAR(0, d.conditional_variance(-1, -1.1, -0.5), 1E-15);
    ASSERT_NEAR((pow(-1-mu11,2)*0.2 + pow(-mu11,2)*0.3) / 0.5, d.conditional_variance(mu11, -1, 1), 1E-15);
}

TEST(DiscreteDistribution, Conditional) {
    const unsigned char a = 10;
	const unsigned char b = 13;
	const size_t dim = (b - a) + 1;
	std::vector<double> p(dim);
	p[0] = 0.25;
	p[1] = 0.4;
	p[2] = 0.34;
    p[3] = 0.01;
	DiscreteDistribution dd(a, p);
    ASSERT_NEAR(p[1] + p[2], dd.range_prob2(11, 13), 1E-15);
    std::unique_ptr<DiscreteDistribution> cond_dd(dd.conditional(11, 13));
    ASSERT_EQ(2u, cond_dd->size());
    ASSERT_EQ(11, cond_dd->lower_bound());
    ASSERT_EQ(12, cond_dd->upper_bound());
    const double sump = p[1] + p[2];
    ASSERT_NEAR(p[1] / sump, cond_dd->prob(11), 1E-15);
    ASSERT_NEAR(p[2] / sump, cond_dd->prob(12), 1E-15);
}

TEST(DiscreteDistribution, OneZero) {
	std::vector<double> p({ 1, 0 });
	DiscreteDistribution d(0, p);
	RNGImpl rng(42);	
	for (size_t i = 0; i < 1000; ++i) {
		ASSERT_EQ(0, d.draw(rng));
	}
}

TEST(DiscreteDistribution, ZeroOne) {
	std::vector<double> p({ 0, 1 });
	DiscreteDistribution d(0, p);
	RNGImpl rng(42);
	for (size_t i = 0; i < 1000; ++i) {
		ASSERT_EQ(1, d.draw(rng));
	}
}

TEST(DiscreteDistribution, DrawFromCDF) {
	const std::vector<double> cdf1({ 0.1, 0.5, 0.9999999 });
	ASSERT_EQ(2u, DiscreteDistribution::draw_from_cdf(cdf1.begin(), cdf1.end(), 1.0));
	ASSERT_EQ(2u, DiscreteDistribution::draw_from_cdf(cdf1.begin(), cdf1.end(), 0.99));
	const std::vector<double> cdf2({ 0.1, 0.5, 1.000001 });
	ASSERT_EQ(2u, DiscreteDistribution::draw_from_cdf(cdf2.begin(), cdf2.end(), 1.0));
	ASSERT_EQ(2u, DiscreteDistribution::draw_from_cdf(cdf2.begin(), cdf2.end(), 0.99));
}

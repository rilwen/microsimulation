/*
(C) Averisera Ltd 2015
*/
#include <gtest/gtest.h>
#include "core/generic_distribution_integral.hpp"
#include <Eigen/Core>
#include <cmath>

using namespace averisera;

TEST(GenericDistributionIntegral,Test) {
	const unsigned char a = 10;
	const unsigned char b = 12;
	const size_t dim = (b - a) + 1;
	std::vector<double> p(dim);
	p[0] = 0.25;
	p[1] = 0.4;
	p[2] = 0.35;
	GenericDistributionIntegral<unsigned char> dd(a, p);
	ASSERT_EQ(dim, dd.size());

    ASSERT_EQ(a, dd.lower_bound());
    ASSERT_EQ(b, dd.upper_bound());

    ASSERT_NEAR(0., dd.range_prob2(4, a), 1E-15);
    ASSERT_NEAR(0., dd.cdf2(a), 1E-15);
    ASSERT_NEAR(0.25, dd.range_prob2(3, a + 1), 1E-15);
    ASSERT_NEAR(0.25, dd.cdf2(a + 1), 1E-15);
    ASSERT_NEAR(0.65, dd.cdf2(a + 2), 1E-15);
    ASSERT_NEAR(0.65, dd.range_prob2(a, b), 1E-15);

	ASSERT_EQ(a, dd.icdf_generic(0));
	ASSERT_EQ(a, dd.icdf_generic(0.1));
	ASSERT_EQ(a, dd.icdf_generic(p[0] - 0.01));
	ASSERT_EQ(a + 1, dd.icdf_generic(p[0]));
	ASSERT_EQ(a + 1, dd.icdf_generic(p[0] + 0.01));
	ASSERT_EQ(a + 1, dd.icdf_generic(p[0] + p[1] - 0.01));
	ASSERT_EQ(b, dd.icdf_generic(p[0] + p[1]));
	ASSERT_EQ(b, dd.icdf_generic(p[0] + p[1] + 0.01));
	ASSERT_EQ(b, dd.icdf_generic(0.99));
	ASSERT_EQ(b, dd.icdf_generic(1));
}

TEST(GenericDistributionIntegral, OneOfMany) {
	GenericDistributionIntegral<unsigned char> dd(0, 2, 4);
	ASSERT_EQ(0, dd.lower_bound());
	ASSERT_EQ(4, dd.upper_bound());
	ASSERT_EQ(0.0, dd.prob(0));
	ASSERT_EQ(0.0, dd.prob(1));
	ASSERT_EQ(1.0, dd.prob(2));
	ASSERT_EQ(0.0, dd.prob(3));
	ASSERT_EQ(0.0, dd.prob(4));
	ASSERT_EQ(0.0, dd.cdf2(0));
	ASSERT_EQ(0.0, dd.cdf2(1));
	ASSERT_EQ(0.0, dd.cdf2(2));
	ASSERT_EQ(1.0, dd.cdf2(3));
	ASSERT_EQ(1.0, dd.cdf2(4));
}

TEST(GenericDistributionIntegral, FromEigen) {
	Eigen::MatrixXd p(2, 2);
	p(0, 0) = 0.4;
	p(1, 0) = 0.6;
	GenericDistributionIntegral<unsigned char> d(0, p.col(0));
	ASSERT_EQ(0, d.lower_bound());
	ASSERT_EQ(1, d.upper_bound());
	ASSERT_EQ(0.4, d.prob(0));
	ASSERT_EQ(0.6, d.prob(1));
}

TEST(GenericDistributionIntegral, MoveConstructors) {
    std::vector<double> p({0.4, 0.6});
    GenericDistributionIntegral<unsigned char> d(1, std::move(p));
    ASSERT_EQ(0u, p.size());
    ASSERT_EQ(1, d.lower_bound());
	ASSERT_EQ(2, d.upper_bound());
    ASSERT_EQ(0.4, d.prob(1));
	ASSERT_EQ(0.6, d.prob(2));
    ASSERT_EQ(2, d.icdf_generic(1.0));
    ASSERT_EQ(1, d.icdf_generic(0.2));
    GenericDistributionIntegral<unsigned char> d2(std::move(d));
    ASSERT_EQ(0u, d.size());
    ASSERT_EQ(1, d2.lower_bound());
	ASSERT_EQ(2, d2.upper_bound());
    ASSERT_EQ(0.4, d2.prob(1));
	ASSERT_EQ(0.6, d2.prob(2));
    ASSERT_EQ(2, d2.icdf_generic(1.0));
    ASSERT_EQ(1, d2.icdf_generic(0.2));
}

TEST(GenericDistributionIntegral, Conditional) {
    const unsigned char a = 10;
	const unsigned char b = 13;
	const size_t dim = (b - a) + 1;
	std::vector<double> p(dim);
	p[0] = 0.25;
	p[1] = 0.4;
	p[2] = 0.34;
    p[3] = 0.01;
	GenericDistributionIntegral<unsigned char> dd(a, p);
    std::unique_ptr<GenericDistributionIntegral<unsigned char>> cond_dd(dd.conditional(11, 13));
    ASSERT_EQ(2u, cond_dd->size());
    ASSERT_EQ(11, cond_dd->lower_bound());
    ASSERT_EQ(12, cond_dd->upper_bound());
    const double sump = p[1] + p[2];
    ASSERT_NEAR(p[1] / sump, cond_dd->prob(11), 1E-15);
    ASSERT_NEAR(p[2] / sump, cond_dd->prob(12), 1E-15);
}

TEST(GenericDistributionIntegral, Mean) {
	unsigned char a = 10;
	unsigned char b = 13;
	const size_t dim = (b - a) + 1;
	std::vector<double> p(dim);
	p[0] = 0.25;
	p[1] = 0.4;
	p[2] = 0.34;
	p[3] = 0.01;
	GenericDistributionIntegral<unsigned char> dd(a, p);
	const double mean_act = GenericDistributionIntegral<unsigned char>::mean(dd);
	const double mean_exp = 0.25 * 10 + 0.4 * 11 + 0.34 * 12 + 0.01 * 13;
	ASSERT_NEAR(mean_exp, mean_act, 1E-14);
	p.resize(256);
	a = 0;
	std::fill(p.begin(), p.end(), 1.0 / 256);
	dd = GenericDistributionIntegral<unsigned char>(a, p);
	ASSERT_NEAR(255.0 / 2, GenericDistributionIntegral<unsigned char>::mean(dd), 1E-12);
}

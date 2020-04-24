#include <gtest/gtest.h>
#include "core/distribution.hpp"
//#include "core/stl_utils.hpp"
#include "testing/rng_precalc.hpp"
#include <cassert>
#include <cmath>

using namespace averisera;

TEST(Distribution, CalcCumProba) {
	std::vector<double> p = { 0.1, 0.4, 0.3, 0.2 };
	std::vector<double> cp(p.size());
	Distribution::calculate_cumulative_proba(p, cp);
	ASSERT_NEAR(0.1, cp[0], 1E-15);
	ASSERT_NEAR(0.5, cp[1], 1E-15);
	ASSERT_NEAR(0.8, cp[2], 1E-15);
	ASSERT_EQ(1.0, cp[3]); // enforced

	p = { 0.1, 0.89 };
	cp.resize(2);
	Distribution::calculate_cumulative_proba(p, cp);
	ASSERT_EQ(1.0, cp[1]);
	Distribution::calculate_cumulative_proba(p, cp, 0.05);
	ASSERT_THROW(Distribution::calculate_cumulative_proba(p, cp, 0.005), std::domain_error);
}

class MockDistribution: public Distribution {
public:
    double cdf(double x) const override {
        if (x < 0) {
            return 0;
        } else if (x > 1) {
            return 1;
        } else {
            return x * x;
        }
    }

    double icdf(double p) const override {
        assert(p >= 0);
        assert(p <= 1);
        return sqrt(p);
    }

    double pdf(double x) const override {
        if (x < 0) {
            return 0;
        } else if (x > 1) {
            return 0;
        } else {
            return 2 * x;
        }
    }

	double infimum() const override {
		return 0;
	}

	double supremum() const override {
		return 1;
	}
};

TEST(Distribution, Draw) {
    MockDistribution md;
    averisera::testing::RNGPrecalc rng({0.25});
    ASSERT_NEAR(0.5, md.draw(rng), 1E-14);
}

TEST(Distribution, AsMultivariate) {
    MockDistribution md;
    ASSERT_EQ(1u, md.dim());
    Eigen::VectorXd x(1);
    Eigen::VectorXd p(1);
    p[0] = 0.25;
    md.marginal_icdf(p, x);
    ASSERT_NEAR(0.5, x[0], 1E-14);
    x[0] = 0.6;
    md.marginal_cdf(x, p);
    ASSERT_NEAR(0.36, p[0], 1E-14);
    Eigen::MatrixXd m(2, 1);
    m << 0.1,
        0.61;
    md.adjust_distribution(m);
    ASSERT_NEAR(0.5, m(0, 0), 1E-14);
    ASSERT_NEAR(sqrt(0.75), m(1, 0), 1E-14);
    m.resize(2,2);
    ASSERT_THROW(md.adjust_distribution(m), std::domain_error);
    m << 0.1, 0.2,
        0.61, 0.2;
    md.adjust_distribution(m.leftCols(1));
    md.adjust_distribution(m.rightCols(1));
    ASSERT_NEAR(0.5, m(0, 0), 1E-14);
    ASSERT_NEAR(sqrt(0.75), m(1, 0), 1E-14);
    averisera::testing::RNGPrecalc rng({0.25, 0.01, 0.04});
    x[0] = -1;
    md.draw(rng, x);
    ASSERT_NEAR(0.5, x[0], 1E-14);
    md.draw_noncont(rng, m.row(0).segment(0, 1));
    ASSERT_NEAR(0.1, m(0, 0), 1E-14);
}

TEST(Distribution, Mean) {
    MockDistribution md;
    ASSERT_NEAR(2.0/3.0, md.mean(), 1E-15);
}

TEST(Distribution, Variance) {
    MockDistribution md;
    ASSERT_NEAR(1.0/18.0, md.variance(2.0/3.0), 1E-15);
}

TEST(Distribution, ConditionalMean) {
    MockDistribution md;
    ASSERT_NEAR(2.0/3.0, md.conditional_mean(0, 1), 1E-15);
    ASSERT_NEAR(0.5, md.conditional_mean(0.5, 0.5 + 1E-15), 1E-15);
    const double a = 0.25;
    const double b = 0.75;
    const double p_ab = b * b - a * a;
    ASSERT_NEAR(2.0 / 3.0 * (pow(b, 3) - pow(a, 3)) / p_ab, md.conditional_mean(a, b), 1E-14);
}

TEST(Distribution, ConditionalVariance) {
    MockDistribution md;
    ASSERT_NEAR(1.0/18.0, md.conditional_variance(2.0/3.0, 0, 1), 1E-15);
    ASSERT_NEAR(0, md.conditional_variance(0.5, 0.5, 0.5 + 1E-15), 1E-15);
    const double a = 0.25;
    const double b = 0.75;
    const double p_ab = b * b - a * a;
    const double mu_ab = 2.0 / 3.0 * (pow(b, 3) - pow(a, 3)) / p_ab;
    const double expected_variance_ab = 11.0 / 1152.0 / p_ab;
    ASSERT_NEAR(expected_variance_ab, md.conditional_variance(mu_ab, 0.25, 0.75), 1E-14);
}

TEST(Distribution, InterpolateContinuousCdf) {
	const std::vector<double> x({ 0., 1., 2. });
	const std::vector<double> cdf({ 0., 0.4, 1.0 });
	const std::vector<double> new_x({ 0., 0.25, 1., 1.5, 2. });
	const std::vector<double> new_cdf_act(Distribution::interpolate_continuous_cdf(x, cdf, new_x));
	const std::vector<double> new_cdf_exp({ 0., 0.1, 0.4, 0.7, 1.0 });
	ASSERT_EQ(new_cdf_exp.size(), new_cdf_act.size());
	ASSERT_EQ(0., new_cdf_act.front());
	ASSERT_EQ(1., new_cdf_act.back());
	for (size_t i = 0; i < new_cdf_exp.size(); ++i) {
		ASSERT_NEAR(new_cdf_exp[i], new_cdf_act[i], 1E-15) << i;
	}
}

TEST(Distribution, MapValuesViaCdfs) {
	const std::vector<double> x({ 0, 10, 20, 30 });
	const std::vector<double> cdf1({ 0, 1. / 3, 2. / 3, 1.0 });
	const double eps = 1e-12;
	const std::vector<double> cdf2({ 0, 0.5 - eps, 0.5 + eps, 1.0 });
	std::vector<double> x1;
	std::vector<double> x2;
	Distribution::map_values_via_cdfs(x, cdf1, cdf2, x1, x2);
	ASSERT_EQ(6, x1.size());
	ASSERT_EQ(x1.size(), x2.size());
	ASSERT_EQ(0, x1[0]);
	ASSERT_EQ(10, x1[1]);
	ASSERT_NEAR(10 * (0.5 - eps) * 3, x1[2], 1E-14);
	ASSERT_NEAR(10 * (0.5 + eps) * 3, x1[3], 1E-14);
	ASSERT_EQ(20, x1[4]);
	ASSERT_EQ(30, x1[5]);
	ASSERT_EQ(0, x2[0]);
	ASSERT_NEAR(10 / (0.5 - eps) / 3, x2[1], 1E-14);
	ASSERT_EQ(10, x2[2]);
	ASSERT_EQ(20, x2[3]);
	ASSERT_NEAR(20 + 10 * (2. / 3 - 0.5 - eps) / (0.5 - eps), x2[4], 1E-14);
	ASSERT_EQ(30, x2[5]);	
	//std::cout << x1 << std::endl;
	//std::cout << x2 << std::endl;
}

TEST(Distribution, IcdfAsFunction) {
	std::function<double(double)> fun;
	{
		std::shared_ptr<const Distribution> distr(new MockDistribution);
		fun = Distribution::icdf_as_function(distr);
	}
	MockDistribution md;
	ASSERT_TRUE(fun);
	ASSERT_EQ(md.icdf(0.5), fun(0.5));
}

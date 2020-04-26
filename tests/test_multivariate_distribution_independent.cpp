// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/distribution.hpp"
#include "core/multivariate_distribution_independent.hpp"
#include "testing/rng_precalc.hpp"

using namespace averisera;

class MockDistribution: public Distribution {
public:
    double cdf(double x) const {
        return x * x;
    }

    double icdf(double p) const {
        return sqrt(p);
    }

    double pdf(double x) const {
        return 2 * x;
    }

	double infimum() const override {
		return 0;
	}

	double supremum() const override {
		return 1;
	}
};

MultivariateDistributionIndependent build(unsigned int dim) {
    std::shared_ptr<const MultivariateDistribution> member = std::make_shared<MockDistribution>();
    return MultivariateDistributionIndependent(std::vector<std::shared_ptr<const MultivariateDistribution>>(dim, member));
}

TEST(MultivariateDistributionIndependent, Draw) {
    MultivariateDistributionIndependent distr = build(2);
    ASSERT_EQ(2u, distr.dim());
    averisera::testing::RNGPrecalc rng({0.25, 0.81}); 
    Eigen::VectorXd v(2);
    distr.draw(rng, v);
    ASSERT_NEAR(0.5, v[0], 1E-14);
    ASSERT_NEAR(0.9, v[1], 1E-14);
    rng.reset();
    std::vector<double> v2(2);
    distr.draw(rng, v2);
    ASSERT_NEAR(0.5, v2[0], 1E-14);
    ASSERT_NEAR(0.9, v2[1], 1E-14);
    Eigen::MatrixXd m(2, 2);
    rng.reset();
    distr.draw_noncont(rng, m.row(0));
    ASSERT_NEAR(0.5, m(0, 0), 1E-14);
    ASSERT_NEAR(0.9, m(0, 1), 1E-14);
}

TEST(MultivariateDistributionIndependent, Marginals) {
    MultivariateDistributionIndependent distr = build(2);
    Eigen::VectorXd x(2);
    Eigen::VectorXd p(2);
    x << 0.1, 0.4;
    distr.marginal_cdf(x, p);
    ASSERT_NEAR(0.01, p[0], 1E-14);
    ASSERT_NEAR(0.16, p[1], 1E-14);
    p << 0.25, 0.81;
    distr.marginal_icdf(p, x);
    ASSERT_NEAR(0.5, x[0], 1E-14);
    ASSERT_NEAR(0.9, x[1], 1E-14);
}

TEST(MultivariateDistributionIndependent, AdjustDistribution) {
    MultivariateDistributionIndependent distr = build(2);
    Eigen::MatrixXd m(2, 2);
    m << 0.1, 0.8,
        0.3, 0.4;
    distr.adjust_distribution(m);
    ASSERT_NEAR(0.5, m(0, 0), 1E-14);
    ASSERT_NEAR(0.5, m(1, 1), 1E-14) << m;
    ASSERT_NEAR(sqrt(0.75), m(1, 0), 1E-14);
    ASSERT_NEAR(sqrt(0.75), m(0, 1), 1E-14);
}

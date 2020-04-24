#include <gtest/gtest.h>
#include "core/discrete_distribution.hpp"
#include "core/eigen.hpp"
#include "core/multivariate_distribution_discrete.hpp"
#include "core/rng_impl.hpp"
#include "core/running_statistics.hpp"

using namespace averisera;

TEST(MultivariateDistributionDiscrete, Test1D) {
    const std::vector<double> probs({0.4, 0.15, 0.45});
    MultivariateDistributionDiscrete md(EigenUtils::from_vec(probs), std::vector<int>({1}), std::vector<int>({3}));
    DiscreteDistribution d(1, probs);
    ASSERT_EQ(1u, md.dim());
    Eigen::VectorXd x(1);
    Eigen::VectorXd p(1);
    x[0] = 2;
    md.marginal_cdf(x, p);
    ASSERT_EQ(p[0], d.cdf(x[0]));
    p[0] = 0.5;
    md.marginal_icdf(p, x);
    ASSERT_EQ(x[0], d.icdf(p[0]));    
}

TEST(MultivariateDistributionDiscrete, TestQuasi1D) {
    const std::vector<double> probs({0.4, 0.15, 0.45});
    MultivariateDistributionDiscrete md(EigenUtils::from_vec(probs), std::vector<int>({1, 0}), std::vector<int>({3, 0}));
    DiscreteDistribution d(1, probs);
    ASSERT_EQ(2u, md.dim());
    Eigen::VectorXd x(2);
    Eigen::VectorXd p(2);
    x[0] = 2;
    x[1] = 0;
    md.marginal_cdf(x, p);
    ASSERT_EQ(p[0], d.cdf(x[0]));
    ASSERT_EQ(p[1], 1.0);
    p[0] = 0.5;
    p[1] = 0.2;
    md.marginal_icdf(p, x);
    ASSERT_EQ(x[0], d.icdf(p[0]));
    ASSERT_EQ(x[1], 0);
}

TEST(MultivariateDistributionDiscrete, Test2D) {
    const std::vector<double> probs({0.1, 0.2, 0.3, 0.4});
    MultivariateDistributionDiscrete md(EigenUtils::from_vec(probs), std::vector<int>({1, 2}), std::vector<int>({2, 3}));
    ASSERT_EQ(2u, md.dim());
    Eigen::VectorXd x(2);
    Eigen::VectorXd p(2);
    x[0] = 2;
    x[1] = 3;
    md.marginal_cdf(x, p);
    ASSERT_EQ(p[0], 1.0);
    ASSERT_EQ(p[1], 1.0);
    x[0] = 1;
    x[1] = 2;
    md.marginal_cdf(x, p);
    ASSERT_NEAR(p[0], 0.4, 1E-15);
    ASSERT_NEAR(p[1], 0.3, 1E-15);
    p[0] = 0.39;
    p[1] = 0.31;
    md.marginal_icdf(p, x);
    ASSERT_EQ(x[0], 1);
    ASSERT_EQ(x[1], 3);
}

TEST(MultivariateDistributionDiscrete, Test2DMatrix) {
    Eigen::MatrixXd probs(2, 2);
    probs << 0.1, 0.2,
        0.3, 0.4;
    MultivariateDistributionDiscrete md(probs, 1, 2);
    ASSERT_EQ(2u, md.dim());
    Eigen::VectorXd x(2);
    Eigen::VectorXd p(2);
    x[0] = 2;
    x[1] = 3;
    md.marginal_cdf(x, p);
    ASSERT_EQ(p[0], 1.0);
    ASSERT_EQ(p[1], 1.0);
    x[0] = 1;
    x[1] = 2;
    md.marginal_cdf(x, p);
    ASSERT_NEAR(p[0], 0.4, 1E-15);
    ASSERT_NEAR(p[1], 0.3, 1E-15);
    p[0] = 0.39;
    p[1] = 0.31;
    md.marginal_icdf(p, x);
    ASSERT_EQ(x[0], 1);
    ASSERT_EQ(x[1], 3);
}

TEST(MultivariateDistributionDiscrete, Draw) {
    const std::vector<double> probs({0.4, 0.15, 0.25, 0.2});
    MultivariateDistributionDiscrete md(EigenUtils::from_vec(probs), std::vector<int>({1, 0}), std::vector<int>({2, 1}));
    const unsigned int iters = 1000;
    std::vector<RunningStatistics<double>> rs(4);
    Eigen::VectorXd sample(2);
    RNGImpl rng(42);
    for (unsigned int n = 0; n < iters; ++n) {
        md.draw(rng, sample);
        const unsigned int idx = static_cast<unsigned int>(sample[1] * 2 + sample[0] - 1);
        for (unsigned int i = 0; i < 4; ++i) {
            rs[i].add(i == idx ? 1.0 : 0.0);
        }
    }
    for (unsigned int i = 0; i < 4; ++i) {
        ASSERT_NEAR(probs[i], rs[i].mean(), rs[i].standard_deviation()) << i;
    }
}

#include <gtest/gtest.h>
#include "core/copula_gaussian.hpp"
#include "core/multivariate_distribution_gaussian.hpp"
#include "core/normal_distribution.hpp"
#include "core/rng_impl.hpp"
#include "core/statistics.hpp"
#include <limits>

using namespace averisera;

TEST(CopulaGaussian, ConditionalSame) {
    const unsigned int dim = 3;
    Eigen::MatrixXd rho(dim, dim);
    rho << 1.0, 0.1, -0.2,
        0.1, 1.0, -0.04,
        -0.2, -0.04, 1.0;
    CopulaGaussian copula(rho);
    std::vector<std::shared_ptr<const Distribution>> marginals(dim);
    for (unsigned int i = 0; i < dim; ++i) {
        marginals[i] = std::make_shared<NormalDistribution>(0, 1);
    }
    Eigen::VectorXd x(dim);
    x << 0.3, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN();
    const std::unique_ptr<MultivariateDistribution> conditional = copula.conditional(marginals.begin(), marginals.end(), x);
    ASSERT_NE(conditional, nullptr);
    const unsigned int n = 10000;
    Eigen::MatrixXd sample(n, dim - 1);
    RNGImpl rng(42);
    for (unsigned int i = 0; i < n; ++i) {
        conditional->draw_noncont(rng, sample.row(i));
    }
    Eigen::MatrixXd sample_cov;
    Statistics::estimate_covariance_matrix(sample, DataCheckLevel::ANY, sample_cov);
    Eigen::MatrixXd expected_cov(dim - 1, dim - 1);
    Eigen::VectorXd expected_mean(dim - 1);
    MultivariateDistributionGaussian::conditional(Eigen::VectorXd::Zero(dim), rho, x, expected_mean, expected_cov);
    ASSERT_NEAR(0.0, (expected_cov - sample_cov).norm(), 3E-2);
    const Eigen::VectorXd sample_mean = sample.colwise().mean();
    ASSERT_NEAR(0.0, (Eigen::VectorXd::Zero(dim - 1) - sample_mean).norm(), 0.1);
}

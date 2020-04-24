#include <gtest/gtest.h>
#include "core/multivariate_distribution_gaussian.cpp"
#include <limits>

using namespace averisera;

TEST(MultivariateDistributionGaussian, Conditional) {
    const unsigned int dim = 3;
    Eigen::MatrixXd cov(dim, dim);
    cov << 2.0, 0.12, 0.1,
        0.12, 0.01, -0.0001,
        0.1, -0.0001, 0.1;
    Eigen::VectorXd mean(dim);
    mean << 0.1,
        -0.4,
        1.4;
    MultivariateDistributionGaussian distr(mean, cov);
    Eigen::MatrixXd cond_cov1(dim - 1, dim - 1), cond_cov2(dim - 1, dim - 1);
    Eigen::VectorXd cond_mean1(dim - 1), cond_mean2(dim - 1);
    Eigen::VectorXd a(dim);
    a << 0.23, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN();
    MultivariateDistributionGaussian::conditional(mean, cov, a, cond_mean1, cond_cov1);
    distr.conditional(a, cond_mean2, cond_cov2);
    ASSERT_EQ(cond_mean1, cond_mean2);
    ASSERT_EQ(cond_cov1, cond_cov2);
    Eigen::VectorXd a2(dim - 1);
    const double x2 = -0.101;
    a2 << std::numeric_limits<double>::quiet_NaN(), x2;
    a[dim - 1] = x2;
    cond_cov2.resize(dim - 2, dim - 2);
    cond_mean2.resize(dim - 2);
    distr.conditional(a, cond_mean2, cond_cov2);
    const double mu = cond_mean2[0];
    const double var = cond_cov2(0, 0);
    MultivariateDistributionGaussian::conditional(cond_mean1, cond_cov1, a2, cond_mean2, cond_cov2);
    const double sigma1 = sqrt(cond_cov1(0,0));
    const double sigma2 = sqrt(cond_cov1(1,1));
    const double rho = cond_cov1(0, 1) / sigma1 / sigma2;
    ASSERT_NEAR(mu, cond_mean1[0] + sigma1 * rho / sigma2 * (a2[1] - cond_mean1[1]), 1E-14) << sigma1 << ", " << sigma2 << ", " << rho << ", " << cond_cov1(0, 1);
    ASSERT_NEAR(var, (1 - rho * rho) * sigma1 * sigma1, 1E-14);
    ASSERT_NEAR(mu, cond_mean2[0], 1E-14);
    ASSERT_NEAR(var, cond_cov2(0, 0), 1E-14);
}

TEST(MultivariateDistributionGaussian, ConditionalIndependent) {
    const unsigned int dim = 3;
    Eigen::MatrixXd cov(dim, dim);
    cov << 2.0, 0, 0,
        0, 0.01, -0.0001,
        0, -0.0001, 0.1;
    Eigen::VectorXd mean(dim);
    mean << 0.1,
        -0.4,
        1.4;
    MultivariateDistributionGaussian distr(mean, cov);
    Eigen::VectorXd a(dim);
    a << 0.23, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN();
    Eigen::MatrixXd cond_cov(dim - 1, dim - 1);
    Eigen::VectorXd cond_mean(dim - 1);
    distr.conditional(a, cond_mean, cond_cov);
    ASSERT_NEAR(0.0, (cond_mean - mean.tail(2)).norm(), 1E-14) << cond_mean;
    ASSERT_NEAR(0.0, (cond_cov - cov.bottomRightCorner(dim - 1, dim - 1)).norm(), 1E-14) << cond_cov;

    cov(0,0) = 0;
    a[0] = mean[0];
    distr.conditional(a, cond_mean, cond_cov);
    ASSERT_NEAR(0.0, (cond_mean - mean.tail(2)).norm(), 1E-14) << cond_mean;
    ASSERT_NEAR(0.0, (cond_cov - cov.bottomRightCorner(dim - 1, dim - 1)).norm(), 1E-14) << cond_cov;
}

TEST(MultivariateDistributionGaussian, ConditionalFullyCorrelated) {
    const unsigned int dim = 3;
    Eigen::MatrixXd cov(dim, dim);
    cov << 4.0, 0.2, -0.8,
        0.2, 0.01, -0.04,
        -0.8, -0.04, 0.16;
    Eigen::VectorXd mean(dim);
    mean << 0.1,
        -0.4,
        1.4;
    MultivariateDistributionGaussian distr(mean, cov);
    Eigen::VectorXd a(dim);
    a << 0.23, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN();
    Eigen::MatrixXd cond_cov(dim - 1, dim - 1);
    Eigen::VectorXd cond_mean(dim - 1);
    distr.conditional(a, cond_mean, cond_cov);
    ASSERT_NEAR(0.0, cond_cov.norm(), 1E-14);
    ASSERT_NEAR(mean[1] + (a[0] - mean[0]) * 0.1 / 2, cond_mean[0], 1E-14);
    ASSERT_NEAR(mean[2] - (a[0] - mean[0]) * 0.4 / 2, cond_mean[1], 1E-14);
}

TEST(MultivariateDistributionGaussian, Construction) {
    const unsigned int dim = 3;
    Eigen::MatrixXd cov(dim, dim);
    cov << 2.0, 0.12, 0.1,
        0.12, 0.01, -0.0001,
        0.1, -0.0001, 0.1;
    Eigen::VectorXd mean(dim);
    mean << 0.1,
        -0.4,
        1.4;
    MultivariateDistributionGaussian distr(mean, cov);
    const auto cov_reconstructed = distr.S() * distr.S().transpose();
    ASSERT_NEAR(0.0, (cov - cov_reconstructed).norm(), 1E-14) << cov_reconstructed;
    ASSERT_NEAR(0.0, (Eigen::MatrixXd::Identity(dim, dim) - distr.S() * distr.invS()).norm(), 1E-14);
    ASSERT_NEAR(0.0, (Eigen::MatrixXd::Identity(dim, dim) - distr.invS() * distr.S()).norm(), 1E-14);
}

#include <gtest/gtest.h>
#include "core/multivariate_distribution_gaussian_simple.hpp"
#include "core/rng_impl.hpp"
#include "core/running_covariance.hpp"

using namespace averisera;

TEST(RunningCovariance, FullCorrelation) {
	RunningCovariance<double> rc;
	RunningStatistics<double> rs;
	for (size_t i = 0; i < 10; ++i) {
		rc.add(-1, -1);
		rc.add(1, 1);
		rs.add(-1);
		rs.add(1);
	}
	ASSERT_EQ(20, rc.nbr_samples());
	ASSERT_NEAR(0.0, rc.meanX(), 1E-16);
	ASSERT_NEAR(0.0, rc.meanY(), 1E-16);
	ASSERT_EQ(rc.varianceX(), rc.varianceY());
	ASSERT_NEAR(rc.varianceX(), rc.covariance(), 1E-15);
	ASSERT_NEAR(rs.variance(), rc.varianceX(), 1E-15);
	ASSERT_NEAR(1.0, rc.correlation(), 1E-15);
}

TEST(RunningCovariance, LargeSymmetricSample) {
	const size_t npath = 200000;
	RNGImpl rng(42);
	Eigen::VectorXd mu(2);
	mu << -0.4, 0.3;
	Eigen::MatrixXd cov(2, 2);
	cov << 3.0, -0.2,
		-0.2, 0.4;
	MultivariateDistributionGaussianSimple distr(mu, cov);
	Eigen::VectorXd sample(2);
	RunningCovariance<double> rc;
	for (size_t i = 0; i < npath; ++i) {
		distr.draw(rng, sample);
		rc.add(sample[0], sample[1]);		
	}
	ASSERT_NEAR(mu[0], rc.meanX(), 6E-3);
	ASSERT_NEAR(mu[1], rc.meanY(), 3E-3);
	ASSERT_NEAR(cov(0, 0), rc.varianceX(), 1E-2);
	ASSERT_NEAR(cov(1, 1), rc.varianceY(), 2E-3);
	ASSERT_NEAR(cov(0, 1), rc.covariance(), 4E-3);
	ASSERT_NEAR(cov(0, 1) / sqrt(cov(0, 0) * cov(1, 1)), rc.correlation(), 4E-3);
}

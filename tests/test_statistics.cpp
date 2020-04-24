/*
(C) Averisera Ltd 2014
*/
#include <gtest/gtest.h>
#include "core/observed_discrete_data.hpp"
#include "core/statistics.hpp"
#include <Eigen/Core>

using namespace averisera;

TEST(Statistics, KLValueAndGradient) {
	std::vector<double> P = { 0.25, 0.3, 0.2, 0.25 };
	std::vector<double> Q = { 0.22, 0.25, 0.25, 0.28 };
	const double kl = Statistics::kl_divergence(P, Q);
	ASSERT_GT(kl, 0.0);
	std::vector<double> grad(P.size());
	const double kl2 = Statistics::kl_divergence_and_gradient(P, Q, grad);
	ASSERT_EQ(kl, kl2);
	const double eps = 1E-8;
	for (size_t i = 0; i < P.size(); ++i) {
		const double q_orig = Q[i];
		Q[i] += eps;
		const double val_hi = Statistics::kl_divergence(P, Q);
		Q[i] -= 2 * eps;
		const double val_lo = Statistics::kl_divergence(P, Q);
		Q[i] = q_orig;
		const double approx_grad = (val_hi - val_lo) / (2 * eps);
		ASSERT_NEAR(grad[i], approx_grad, 2 * eps) << i;
	}
}

TEST(Statistics, KLPointers) {
	std::vector<double> P = { 0.25, 0.3, 0.2, 0.25 };
	std::vector<double> Q = { 0.22, 0.25, 0.25, 0.28 };
	const double kl1 = Statistics::kl_divergence(P, Q);
	const double kl2 = Statistics::kl_divergence(&P[0], &Q[0], P.size());
	ASSERT_EQ(kl1, kl2);
}

TEST(Statistics, KLWeighted) {
	std::vector<double> P = { 0.25, 0.3, 0.2, 0.25 };
	std::vector<double> Q = { 0.22, 0.25, 0.25, 0.28 };
	const double nsampl = 1000.;
	const double kl1 = nsampl * Statistics::kl_divergence(P, Q);
	const double kl2 = Statistics::weighted_kl_divergence(nsampl, P, Q);
	ASSERT_EQ(kl1, kl2);
}

TEST(Statistics, PercentilesInplace) {
    std::vector<double> x = { 10, 2, 5, 15, 1 };
    Statistics::percentiles_inplace(x.begin(), x.end());
    ASSERT_EQ(std::vector<double>({ 0.7, 0.3, 0.5, 0.9, 0.1 }), x);
    
    Eigen::MatrixXd m(2, 2);
    m << 1, 2,
        2, 1;
    Statistics::percentiles_inplace(m);
    ASSERT_EQ(0.25, m(0, 0));
    ASSERT_EQ(0.75, m(1, 0));
    ASSERT_EQ(0.75, m(0, 1));
    ASSERT_EQ(0.25, m(1, 1));
}

TEST(Statistics, EstimateCovarianceMatrix) {
    Eigen::MatrixXd data(3, 2);
    data << 0.4, 0.2,
        1.2, 0.9,
        -0.4, -0.32;
    Eigen::MatrixXd cov;
    Eigen::MatrixXd expected_cov(2, 2);
    expected_cov << 0.64, 0.488,
        0.488, 0.3748;
    Statistics::estimate_covariance_matrix(data, DataCheckLevel::ANY, cov);
    ASSERT_NEAR(0, (cov - expected_cov).norm(), 1E-15) << cov;
    
    Eigen::VectorXd means(2);
    means << 0, 0;
    Statistics::estimate_covariance_matrix(data, means, DataCheckLevel::ANY, cov);
    expected_cov << 1.76/3, 1.288/3,
        1.288/3, 0.9524/3;
    ASSERT_NEAR(0, (cov - expected_cov).norm(), 1E-14) << cov;
}

static double ref_sum_squared_differences_pairwise(const std::vector<double>& v) {
	double sum = 0;
	for (auto it1 = v.begin(); it1 != v.end(); ++it1) {
		const double x1 = *it1;
		for (auto it2 = v.begin(); it2 != it1; ++it2) {
			sum += pow(*it2 - x1, 2);
		}
	}
	return sum;
}

TEST(Statistics, sum_squared_differences_pairwise) {
	std::vector<double> v({ 0.5, 1.5 });
	const double tol = 1e-12;
	ASSERT_NEAR(ref_sum_squared_differences_pairwise(v), Statistics::sum_squared_differences_pairwise(v), tol);
	v.push_back(1.0);
	ASSERT_NEAR(ref_sum_squared_differences_pairwise(v), Statistics::sum_squared_differences_pairwise(v), tol);
	v.push_back(10.9);
	ASSERT_NEAR(ref_sum_squared_differences_pairwise(v), Statistics::sum_squared_differences_pairwise(v), tol);
	v.push_back(-5.7);
	ASSERT_NEAR(ref_sum_squared_differences_pairwise(v), Statistics::sum_squared_differences_pairwise(v), tol);
}

TEST(Statistics, Median) {
    std::vector<double> v({3., -1., 10.});
	ASSERT_EQ(3.0, Statistics::median(v));
    v = {3., -1., 3.1, 10.};
	ASSERT_NEAR(3.05, Statistics::median(v), 1e-10);
    v = {10.};
	ASSERT_EQ(10.0, Statistics::median(v));
	v.clear();
	ASSERT_THROW(Statistics::median(v), std::domain_error);
}

// TODO: test standard_deviations_delta

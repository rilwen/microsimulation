/*
(C) Averisera Ltd
*/
#include <gtest/gtest.h>
#include "core/ols.hpp"

using namespace averisera;

TEST(OLS, OneDim) {
	OLS ols;
	ols.calculate_residuals(true);
	ols.fit_intercept(true);
	ols.calculate_metrics(true);
	ols.calculate_coefficient_covariance_matrix(true);
	std::vector<double> x({ 0, 1, 2, 3, 4, 5 });
	const double eps = 0.01;
	std::vector<double> y({ 1 - eps, 1.1 + eps, 1.2 - eps, 1.3 + eps, 1.4 - eps, 1.5 + eps });
	const Eigen::VectorXd y2(Eigen::Map<const Eigen::VectorXd>(&y[0], y.size()));
	ASSERT_TRUE(ols.empty());
	ols.fit(x, y);
	ASSERT_FALSE(ols.empty());
	const double ssr = ols.residuals().squaredNorm();
	ASSERT_LE(ssr, static_cast<double>(y.size()) * eps * eps);
	EXPECT_NEAR(1.0, ols.b(), 5e-3);
	ASSERT_EQ(1, ols.a().size());
	ASSERT_EQ(y.size(), ols.residuals().size());
	ASSERT_NEAR(0.1, ols.a()[0], 2e-3);
	ASSERT_LE(ols.adj_r2(), ols.r2());
	ASSERT_NEAR(ssr, ols.ssr(), 1e-10);
	ASSERT_NEAR(Eigen::VectorXd(y2.array() - y2.mean()).squaredNorm(), ols.sst(), 1e-10);
	const double aic = ols.aic();
	const double bic = ols.bic();
	ASSERT_LT(bic, ols.empty_bic());
	ASSERT_EQ(2, ols.inverse_result_covariance().rows());
	ASSERT_EQ(2, ols.inverse_result_covariance().cols());
	ASSERT_EQ(2, ols.result_covariance().rows());
	ASSERT_EQ(2, ols.result_covariance().cols());
	ASSERT_NEAR(0., (ols.result_covariance() * ols.inverse_result_covariance() - Eigen::MatrixXd::Identity(2, 2)).norm(), 1e-10) << ols.result_covariance() * ols.inverse_result_covariance();
	ASSERT_GE(ols.result_covariance().diagonal().minCoeff(), 0.0) << ols.result_covariance();

	ols.fit_intercept(false);
	ols.fit(x, y);
	const double ssr2 = ols.residuals().squaredNorm();
	ASSERT_GE(ssr2, static_cast<double>(y.size()) * eps * eps);
	ASSERT_EQ(1, ols.a().size());
	ASSERT_EQ(y.size(), ols.residuals().size());
	ASSERT_EQ(0.0, ols.b());
	ASSERT_GT(ols.a()[0], 0.1);
	ASSERT_NEAR(1.5 / 5, ols.a()[0], 0.1);
	ASSERT_LE(ols.adj_r2(), ols.r2());
	ASSERT_NEAR(ssr2, ols.ssr(), 1e-10);
	ASSERT_NEAR(y2.squaredNorm(), ols.sst(), 1e-10);
	ASSERT_GE(ols.r2(), 0.0);
	ASSERT_GE(ols.adj_r2(), 0.0);
	ASSERT_GT(ols.aic(), aic) << "AIC should get worse without intercept";
	ASSERT_GT(ols.bic(), bic) << "BIC should get worse without intercept";
	const double bic2 = ols.bic();
	const double aic2 = ols.aic();
	const double empty_bic2 = ols.empty_bic();
	const double sst2 = ols.sst();
	const auto a2 = ols.a();
	const auto res2 = ols.residuals();
	const double r2_2 = ols.r2();
	const double adj_r2_2 = ols.adj_r2();

	OLS ols2;
	ols2 = std::move(ols);
	ASSERT_FALSE(ols2.empty());
	ASSERT_EQ(ssr2, ols2.ssr());
	ASSERT_EQ(sst2, ols2.sst());
	ASSERT_EQ(aic2, ols2.aic());
	ASSERT_EQ(bic2, ols2.bic());
	ASSERT_EQ(empty_bic2, ols2.empty_bic());
	ASSERT_EQ(0.0, ols.b());
	ASSERT_EQ(0.0, (ols2.a() - a2).norm());
	ASSERT_EQ(0.0, (ols2.residuals() - res2).norm());
	ASSERT_EQ(r2_2, ols2.r2());
	ASSERT_EQ(adj_r2_2, ols2.adj_r2());
}

TEST(OLS, TwoDim) {
	OLS ols;
	ols.calculate_residuals(true);
	ols.fit_intercept(true);
	ols.calculate_metrics(true);
	const Eigen::Index n = 10;
	const Eigen::Index d = 2;
	Eigen::MatrixXd X(n, d);
	Eigen::VectorXd y(n);
	const double a0 = -0.25;
	const double a1 = 0.6;
	const double b = -0.1;
	for (Eigen::Index i = 0; i < n; ++i) {
		X(i, 0) = std::sin(i);
		X(i, 1) = static_cast<double>(i * i) - 20;
		y[i] = a0 * X(i, 0) + a1 * X(i, 1) + b;
	}
	ols.fit(X, y);
	ASSERT_EQ(n, ols.residuals().size());
	ASSERT_EQ(n, ols.prediction().size());
	ASSERT_EQ(d, ols.a().size());
	ASSERT_NEAR(b, ols.b(), 1e-10);
	ASSERT_NEAR(a0, ols.a()[0], 1e-10);
	ASSERT_NEAR(a1, ols.a()[1], 1e-10);
	ASSERT_NEAR(0.0, ols.ssr(), 1e-10);
	ASSERT_NEAR(Eigen::VectorXd(y.array() - y.mean()).squaredNorm(), ols.sst(), 1e-10);
	ASSERT_NEAR(0.0, ols.residuals().norm(), 1e-10);
	ASSERT_NEAR(0.0, (ols.prediction() - y).norm(), 1e-10);
	ASSERT_NEAR(1.0, ols.r2(), 1e-10);
	ASSERT_NEAR(1.0, ols.adj_r2(), 1e-10);
	ASSERT_LT(ols.bic(), ols.empty_bic() - 3);
	const double aic = ols.aic();
	const double bic = ols.bic();

	// fit 1 factor only
	ols.fit(X.block(0, 0, n, 1), y);
	ASSERT_GT(ols.aic() - 2, aic) << "AIC should get worse with 1 factor";
	ASSERT_GT(ols.bic() - 2, bic) << "BIC should get worse with 1 factor";
}

TEST(OLS, BICComparator) {
	const auto cmp = OLS::make_bic_comparator(6);

	const Eigen::Index n = 10;
	const Eigen::Index d = 2;
	Eigen::MatrixXd X(n, d);
	Eigen::VectorXd y(n);
	const double a0 = -0.25;
	const double a1 = 0.6;
	const double b = -0.1;
	for (Eigen::Index i = 0; i < n; ++i) {
		X(i, 0) = std::sin(i);
		X(i, 1) = static_cast<double>(i * i) - 20;
		y[i] = a0 * X(i, 0) + a1 * X(i, 1) + b;
	}
	
	OLS ols1;
	ols1.fit_intercept(true);
	ols1.calculate_metrics(true);
	ols1.fit(X, y);
	ASSERT_LT(ols1.bic(), ols1.empty_bic());

	OLS ols2;
	ols2.fit_intercept(true);
	ols2.calculate_metrics(true);

	ASSERT_TRUE(cmp(ols2, ols1));

	ols2.fit(X.block(0, 0, n, 1), y);
	ASSERT_TRUE(cmp(ols2, ols1));
}

TEST(OLS, factor_rank_res_adj_r2) {
	const Eigen::Index n = 10;
	const Eigen::Index d = 2;
	Eigen::MatrixXd X(n, d);
	Eigen::VectorXd y(n);
	const double a0 = -0.25;
	const double a1 = 0.6;
	const double b = -0.1;
	for (Eigen::Index i = 0; i < n; ++i) {
		X(i, 0) = std::sin(i);
		X(i, 1) = static_cast<double>(i * i) - 20;
		y[i] = a0 * X(i, 0) + a1 * X(i, 1) + b;
	}

	OLS ols;
	ols.fit_intercept(true);
	double rank = OLS::factor_rank_res_adj_r2(X, y, ols, std::vector<size_t>(), 0);
	EXPECT_LE(rank, 1.0);
	EXPECT_GE(rank, -0.1);
	rank = OLS::factor_rank_res_adj_r2(X, y, ols, std::vector<size_t>(), 1);
	EXPECT_LE(rank, 1.0);
	EXPECT_GE(rank, -0.1);

	ols.calculate_residuals(true);
	ols.fit(X.col(1), y);
	ASSERT_NEAR(a1, ols.a()[0], 1e-2);
	rank = OLS::factor_rank_res_adj_r2(X, y, ols, std::vector<size_t>(), 0);
	EXPECT_LE(rank, 1.0);
	EXPECT_GE(rank, -0.1);
}

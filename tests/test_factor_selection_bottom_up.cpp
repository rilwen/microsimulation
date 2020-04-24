/*
(C) Averisera Ltd 2017
*/
#include <gtest/gtest.h>
#include "core/factor_selection_bottom_up.hpp"
#include "core/ols.hpp"
#include <array>

using namespace averisera;

TEST(FactorSelectionBottomUp, OLS) {
	const auto fs = make_factor_selection_bottom_up(OLS::make_model_factory(true), OLS::factor_rank_res_adj_r2, OLS::make_bic_comparator());
	const Eigen::Index n = 10;
	const Eigen::Index d = 4;
	Eigen::MatrixXd X(n, d);
	Eigen::VectorXd y(n);
	const std::array<double, d> a = { 0, -0.25, 0.6, 0 };
	const double b = -0.1;
	for (Eigen::Index i = 0; i < n; ++i) {
		X(i, 0) = static_cast<double>(i * i * i) - 20;
		X(i, 1) = std::sin(i); 
		X(i, 2) = - static_cast<double>(i) + 0.3;
		X(i, 3) = std::cos(i * i);
		y[i] = b;
		for (size_t j = 0; j < d; ++j) {
			y[i] += a[j] * X(i, j);
		}
	}
	const auto selected = fs.select(X, y);
	ASSERT_EQ(std::vector<size_t>({ 1, 2 }), selected);
}

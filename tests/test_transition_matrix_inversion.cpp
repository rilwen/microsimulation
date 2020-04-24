/*
(C) Averisera Ltd 2015
*/
#include <gtest/gtest.h>
#include "core/transition_matrix_inversion.hpp"
#include <Eigen/Dense>

TEST(TransitionMatrixInversion, Memory1) {
	const unsigned int dim = 4;
	Eigen::MatrixXd pi(dim, dim);
	pi << 0.81807026162739,	0,	0.99999999981448,	0,
		0.18192973837261,	0,	1.8551795355514e-010,	0,
		0, 0.22785101338448,	0,	0.030183054709089,
		0, 0.77214898661552,	0,	0.96981694529091;
	Eigen::VectorXd y(dim);
	y << 0.27154810055918,	0.72845189928931,	1.0960195148749e-010,	4.190937313135e-011;
	Eigen::VectorXd x = averisera::TransitionMatrixInversion::apply_inverse_pi(pi, y, 1E-8);
	Eigen::VectorXd r = pi*x - y;
	ASSERT_NEAR(x.sum(), 1., 1E-12);
	for (unsigned int i = 0; i < dim; ++i) {
		ASSERT_TRUE(x[i] >= 0) << i;
	}
}

TEST(TransitionMatrixInversion, Memory0) {
	const unsigned int dim = 2;
	Eigen::MatrixXd pi(dim, dim);
	pi << 0.88783824467837,	0.027168683739576,
		0.11216175532163,	0.97283131626042;
	Eigen::VectorXd y(dim);
	y << 0.39232060896013,	0.60767939103987;
	const Eigen::VectorXd x = averisera::TransitionMatrixInversion::apply_inverse_pi(pi, y, 1E-8);
	const Eigen::VectorXd r = pi*x - y;
	ASSERT_NEAR(x.sum(), 1., 1E-12);
	for (unsigned int i = 0; i < dim; ++i) {
		ASSERT_TRUE(x[i] >= 0) << i;
	}
	Eigen::VectorXd x2(pi.inverse() * y);
	EXPECT_NEAR(r.squaredNorm(), 0, 1E-8);
	EXPECT_NEAR((x2 - x).norm(), 0, 1E-8);
}

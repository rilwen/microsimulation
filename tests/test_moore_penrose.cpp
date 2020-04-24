/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include <gtest/gtest.h>
#include "core/moore_penrose.hpp"

TEST(MoorePenrose, Square) {
	Eigen::MatrixXd M(2, 2);
	M.setConstant(0.25);
	Eigen::MatrixXd inverse;
	averisera::MoorePenrose::inverse(M, 1E-12, inverse);
	Eigen::MatrixXd expected_inverse(2, 2);
	expected_inverse.setConstant(1.0);
	ASSERT_NEAR(0.0, (inverse - expected_inverse).norm(), 1E-15);

    M << 1.0, 0.2,
        -0.1, 0.9;
    averisera::MoorePenrose::inverse(M, 1E-12, inverse);
    ASSERT_NEAR(0.0, (Eigen::MatrixXd::Identity(2, 2) - inverse * M).norm(), 1E-15);
    ASSERT_NEAR(0.0, (Eigen::MatrixXd::Identity(2, 2) -  M * inverse).norm(), 1E-15);
}


TEST(MoorePenrose, MoreRows) {
    Eigen::MatrixXd M(4, 2);
    M.setConstant(0.25);
    Eigen::MatrixXd inverse;
    averisera::MoorePenrose::inverse(M, 1E-12, inverse);
    ASSERT_EQ(2, inverse.rows());
    ASSERT_EQ(4, inverse.cols());
}

TEST(MoorePenrose, MoreCols) {
    Eigen::MatrixXd M(2, 4);
    M.setConstant(0.25);
    Eigen::MatrixXd inverse;
    averisera::MoorePenrose::inverse(M, 1E-12, inverse);
    ASSERT_EQ(4, inverse.rows());
    ASSERT_EQ(2, inverse.cols());
}

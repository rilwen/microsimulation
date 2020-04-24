/*
 * (C) Averisera Ltd 2015
 */

#include <gtest/gtest.h>
#include "core/copula_alphastable.hpp"

using namespace averisera;

TEST(CopulaGaussianGeneric, SingleMode) {
    Eigen::MatrixXd corr(2, 2);
    corr << 1, 1,
            1, 1;
    CopulaAlphaStable c1(corr, 0.6, 1);
    ASSERT_EQ(2u, c1.dim());
    ASSERT_EQ(1u, c1.nbr_independent_factors());
    ASSERT_NEAR(1.0, c1.S()(0, 0), 1E-15);
    ASSERT_NEAR(1.0, c1.S()(1, 0), 1E-15);
    CopulaAlphaStable c2(corr, 1, 1);
    ASSERT_EQ(2u, c2.dim());
    ASSERT_EQ(1u, c2.nbr_independent_factors());
}

TEST(CopulaGaussianGeneric, TwoModes) {
    Eigen::MatrixXd corr(2, 2);
    corr << 1, 0.6,
            0.6, 1;
    CopulaAlphaStable c1(corr, 0.45, 1);
    ASSERT_EQ(2u, c1.dim());
    ASSERT_EQ(1u, c1.nbr_independent_factors());
    ASSERT_NEAR(1.0, c1.S()(0, 0), 1E-15);
    ASSERT_NEAR(1.0, c1.S()(1, 0), 1E-15);
    CopulaAlphaStable c2(corr, 1, 0);
    ASSERT_EQ(2u, c2.dim());
    ASSERT_EQ(2u, c2.nbr_independent_factors());
    ASSERT_NEAR(sqrt(0.8), c2.S()(0, 0), 1E-15);
    ASSERT_NEAR(sqrt(0.8), c2.S()(1, 0), 1E-15);
    ASSERT_NEAR(-sqrt(0.2), c2.S()(0, 1), 1E-15);
    ASSERT_NEAR(sqrt(0.2), c2.S()(1, 1), 1E-15);
    CopulaAlphaStable c3(corr, 0.5, 0);
    ASSERT_EQ(2u, c3.dim());
    ASSERT_EQ(1u, c3.nbr_independent_factors());
    ASSERT_THROW(CopulaAlphaStable(corr, 0.95, 1), std::runtime_error);
}

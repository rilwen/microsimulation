#include <gtest/gtest.h>
#include "core/copula_alphastable.hpp"

using namespace averisera;

TEST(CopulaGaussian1Factor, Construct) {
    const std::vector<double> beta = {-0.5, 0.2};
    CopulaAlphaStable c(2, beta);
    ASSERT_EQ(2u, c.dim());
    ASSERT_EQ(2u, static_cast<unsigned int>(c.S().rows()));
    ASSERT_EQ(3u, static_cast<unsigned int>(c.S().cols()));
    ASSERT_EQ(3u, c.nbr_independent_factors());
    for (unsigned int i = 0; i < 2; ++i) {
        ASSERT_EQ(beta[i], c.S()(i, i)) << i;
    }
}

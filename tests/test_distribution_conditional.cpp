#include <gtest/gtest.h>
#include "core/distribution_conditional.hpp"
#include "core/discrete_distribution.hpp"
#include "core/normal_distribution.hpp"

using namespace averisera;

TEST(DistributionConditional, Test) {
    const std::vector<double> probs = {0.2, 0.3, 0.4, 0.1};
    const int a = 0;
    const auto orig = std::make_shared<DiscreteDistribution>(a, probs);
    auto cond = std::make_shared<DistributionConditional>(orig, 0.5, 2.5);
    ASSERT_NEAR(0.7, cond->prob(), 1E-15);
    ASSERT_NEAR(1.0, cond->range_prob(0.6, 2.2), 1E-15);
    ASSERT_NEAR(1.0, cond->range_prob2(0.6, 2.2), 1E-15);
    ASSERT_NEAR(0.4 / 0.7, cond->range_prob(1, 2), 1E-15);
    ASSERT_NEAR(0.3 / 0.7, cond->range_prob2(1, 2), 1E-15);
    ASSERT_NEAR(0.3 / 0.7, cond->cdf(1), 1E-15);
    ASSERT_EQ(0.0, cond->cdf2(1));
    ASSERT_NEAR(1.0, cond->cdf(2), 1E-15);
    ASSERT_NEAR(0.3 / 0.7, cond->cdf2(2), 1E-15);
    ASSERT_NEAR(1.0, cond->icdf(0), 1E-15);
    ASSERT_NEAR(1.0, cond->icdf(0.29), 1E-15);
    ASSERT_NEAR(1.0, cond->icdf(0.3 / 0.7 - 1E-12), 1E-15);
    ASSERT_NEAR(2.0, cond->icdf(0.3 / 0.7 + 1E-12), 1E-15);
    ASSERT_NEAR(2.0, cond->icdf(0.8), 1E-15);
    ASSERT_NEAR(2.0, cond->icdf(1), 1E-15);
    ASSERT_NEAR(orig->conditional_mean(0.5, 2.5), cond->mean(), 1E-15);
    ASSERT_NEAR(orig->conditional_variance(cond->mean(), 0.5, 2.5), cond->variance(cond->mean()), 1E-15);

    cond = std::make_shared<DistributionConditional>(orig, 1, 2);
    ASSERT_NEAR(1.0, cond->mean(), 1E-15);
    ASSERT_NEAR(0.0, cond->variance(1), 1E-15);
}

TEST(DistributionConditional, PDF) {
    const auto orig = std::make_shared<NormalDistribution>();
    auto cond = std::make_shared<DistributionConditional>(orig, -1, 2);
    const double pab = NormalDistribution::normcdf(2) - NormalDistribution::normcdf(-1);
    ASSERT_NEAR(orig->pdf(0.4) / pab, cond->pdf(0.4), 1E-15);
    ASSERT_EQ(0, cond->pdf(-3));
    ASSERT_EQ(0, cond->pdf(3));
    ASSERT_EQ(0, cond->cdf(-3));
    ASSERT_NEAR(1.0, cond->cdf(3), 1E-15);
}

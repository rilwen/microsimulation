// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/generic_distribution_enumerated.hpp"

using namespace averisera;

TEST(GenericDistributionEnumerated, Bool) {
    GenericDistributionEnumerated<bool> distr({false, true}, {0.8, 0.2});
    ASSERT_FALSE(distr.lower_bound());
    ASSERT_TRUE(distr.upper_bound());
    ASSERT_EQ(0., distr.cdf2(false));
    ASSERT_EQ(0.8, distr.cdf2(true));
    ASSERT_EQ(0.8, distr.range_prob2(false, true));
    ASSERT_EQ(2u, distr.size());
    ASSERT_TRUE(false == distr.icdf_generic(0.4));
    ASSERT_TRUE(true == distr.icdf_generic(0.81));
    std::unique_ptr<GenericDistributionEnumerated<bool>> cond_distr(distr.conditional(false, true));
    ASSERT_TRUE(false == cond_distr->lower_bound());
    ASSERT_TRUE(false == cond_distr->upper_bound());
    ASSERT_EQ(1u, cond_distr->size());
    ASSERT_NEAR(1.0, cond_distr->range_prob2(false, true), 1E-15);    
}

TEST(GenericDistributionEnumerated, Int) {
	const std::vector<int> values({ -10, 2, 3, 4, 10 });
	const std::vector<double> probs({ 0.2, 0.1, 0.15, 0.25, 0.3 });
    GenericDistributionEnumerated<int> distr(values, probs);
    ASSERT_EQ(-10, distr.lower_bound());
    ASSERT_EQ(10, distr.upper_bound());
    ASSERT_EQ(0., distr.cdf2(-10));
    ASSERT_EQ(0.2, distr.cdf2(-9));
    ASSERT_EQ(0.2, distr.cdf2(2));
    ASSERT_EQ(5u, distr.size());
    ASSERT_NEAR(0.3, distr.cdf2(3), 1E-15);
    ASSERT_NEAR(0.45, distr.cdf2(4), 1E-15);
    ASSERT_NEAR(0.7, distr.cdf2(5), 1E-15);
    ASSERT_NEAR(0.7, distr.cdf2(10), 1E-15);
    ASSERT_NEAR(1.0, distr.cdf2(11), 1E-15);
    ASSERT_NEAR(0.5, distr.range_prob2(2, 5), 1E-15);
    ASSERT_EQ(-10, distr.icdf_generic(0));
    ASSERT_EQ(10, distr.icdf_generic(1));
    ASSERT_EQ(-10, distr.icdf_generic(0.1));
    ASSERT_EQ(10, distr.icdf_generic(0.8));
    ASSERT_EQ(4, distr.icdf_generic(0.69));
    std::unique_ptr<GenericDistributionEnumerated<int>> cond_distr(distr.conditional(1, 4));
    ASSERT_EQ(2u, cond_distr->size());
    ASSERT_EQ(2, cond_distr->lower_bound());
    ASSERT_EQ(3, cond_distr->upper_bound());
    ASSERT_NEAR(0.1 / 0.25, cond_distr->cdf2(3), 1E-15);
    ASSERT_NEAR(1.0, cond_distr->cdf2(4), 1E-15);
    ASSERT_NEAR(0.15 / 0.25, cond_distr->range_prob2(3, 10), 1E-15);

	std::vector<int> reverse_values(values);
	std::reverse(reverse_values.begin(), reverse_values.end());
	std::vector<double> reverse_probs(probs);
	std::reverse(reverse_probs.begin(), reverse_probs.end());
	const auto distr2 = GenericDistributionEnumerated<int>::from_unsorted(reverse_values, reverse_probs);
	for (int v : values) {
		ASSERT_EQ(distr.cdf2(v), distr2->cdf2(v)) << v;
	}
}

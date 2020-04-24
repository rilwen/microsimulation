#include <gtest/gtest.h>
#include <cmath>
#include <limits>

TEST(Math, Exp) {
    ASSERT_EQ(std::exp(-std::numeric_limits<double>::infinity()), 0.0);
    ASSERT_EQ(std::exp(-std::numeric_limits<long double>::infinity()), 0.0L);
    ASSERT_EQ(std::exp(-std::numeric_limits<float>::infinity()), 0.0F);
    ASSERT_EQ(exp(-std::numeric_limits<double>::infinity()), 0.0);
    ASSERT_EQ(exp(-std::numeric_limits<long double>::infinity()), 0.0L);
    ASSERT_EQ(exp(-std::numeric_limits<float>::infinity()), 0.0F);

    ASSERT_EQ(std::exp(std::numeric_limits<double>::infinity()), std::numeric_limits<double>::infinity());
    ASSERT_EQ(std::exp(std::numeric_limits<long double>::infinity()), std::numeric_limits<long double>::infinity());
    ASSERT_EQ(std::exp(std::numeric_limits<float>::infinity()), std::numeric_limits<float>::infinity());
    ASSERT_EQ(exp(std::numeric_limits<double>::infinity()), std::numeric_limits<double>::infinity());
    ASSERT_EQ(exp(std::numeric_limits<long double>::infinity()), std::numeric_limits<long double>::infinity());
    ASSERT_EQ(exp(std::numeric_limits<float>::infinity()), std::numeric_limits<float>::infinity());
}
    

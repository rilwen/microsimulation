#include "microsim-simulator/feature_user.hpp"
#include <gtest/gtest.h>
#include <string>

using namespace averisera;
using namespace averisera::microsim;

TEST(FeatureUser, Combine) {
    std::unordered_set<std::string> first({"aaa", "bbb"});
    std::unordered_set<std::string> second({"bbb", "bb", "ccc"});
    std::unordered_set<std::string> expected({"aaa", "bb", "bbb", "ccc"});
    ASSERT_EQ(expected, FeatureUser<std::string>::combine(first, second));
}
// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/feature.hpp"
#include <sstream>

using namespace averisera;
using namespace averisera::microsim;

TEST(Feature, InputChecks) {
    ASSERT_THROW(Feature(std::string("")), std::domain_error);
    ASSERT_THROW(Feature(""), std::domain_error);
    ASSERT_THROW(Feature(nullptr), std::domain_error);
}

TEST(Feature, Default) {
    Feature f;
    std::stringstream ss;
    ss << f;
    ASSERT_EQ("", ss.str());
}

TEST(Feature, Output) {
    std::stringstream ss;
    Feature f("dandy");
    ss << f;
    ASSERT_EQ("dandy", ss.str());
}

TEST(Feature, Comparison) {
    ASSERT_EQ(Feature("dandy"), Feature(std::string("dandy")));
}

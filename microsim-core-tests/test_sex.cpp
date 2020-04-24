#include <gtest/gtest.h>
#include "microsim-core/sex.hpp"
#include <sstream>

using namespace averisera;
using namespace averisera::microsim;

TEST(Sex, Print) {
    std::stringstream ss;
    ss << Sex::FEMALE;
    ASSERT_EQ("FEMALE", ss.str());
    ss.str("");
    ss << Sex::MALE;
    ASSERT_EQ("MALE", ss.str());
}

TEST(Sex, FromString) {
    ASSERT_EQ(Sex::FEMALE, sex_from_string("FEMALE"));
    ASSERT_EQ(Sex::MALE, sex_from_string("MALE"));
    ASSERT_THROW(sex_from_string("FOO"), std::runtime_error);
}

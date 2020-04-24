/*
(C) Averisera Ltd 2017
*/
#include "core/string_utils.hpp"
#include <gtest/gtest.h>

using namespace averisera;

TEST(StringUtils, Empty) {
	ASSERT_EQ(std::string(), StringUtils::join(std::vector<std::string>(), ", "));
}

TEST(StringUtils, Single) {
	ASSERT_EQ(std::string("ala"), StringUtils::join(std::vector<std::string>({ "ala" }), ", "));
}

TEST(StringUtils, Multiple) {
	ASSERT_EQ(std::string("ala ma kota"), StringUtils::join(std::vector<std::string>({ "ala", "ma", "kota" }), " "));
}

// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/utils.hpp"
#include "core/period_type.hpp"

using namespace averisera;


TEST(PeriodType, Output) {
    ASSERT_EQ("NONE", Utils::to_string(PeriodType::NONE));
    ASSERT_EQ("D", Utils::to_string(PeriodType::DAYS));
    ASSERT_EQ("W", Utils::to_string(PeriodType::WEEKS));
    ASSERT_EQ("M", Utils::to_string(PeriodType::MONTHS));
    ASSERT_EQ("Y", Utils::to_string(PeriodType::YEARS));    
}

TEST(PeriodType, FromString) {
	ASSERT_EQ(PeriodType::NONE, period_type_from_string(std::string("NONE").c_str()));
	ASSERT_EQ(PeriodType::DAYS, period_type_from_string("D"));
	ASSERT_EQ(PeriodType::WEEKS, period_type_from_string("W"));
	ASSERT_EQ(PeriodType::MONTHS, period_type_from_string("M"));
	ASSERT_EQ(PeriodType::YEARS, period_type_from_string("Y"));
	ASSERT_THROW(period_type_from_string("foo"), std::runtime_error);
}
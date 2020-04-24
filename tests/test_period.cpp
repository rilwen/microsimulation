#include <gtest/gtest.h>
#include "core/utils.hpp"
#include "core/period.hpp"

using namespace averisera;

TEST(Period, Test) {
    Period p = {PeriodType::NONE, 0};
    ASSERT_EQ(0, p.days());
    Period p2 = { PeriodType::DAYS, -6 };
    ASSERT_EQ(-6, p2.days());
    Period p3 = { PeriodType::WEEKS, 2 };
    ASSERT_EQ(14, p3.days());
    Period p4 = { PeriodType::MONTHS, -2 };
    ASSERT_EQ(-60, p4.days());
    Period p5 = { PeriodType::YEARS, 1 };
    ASSERT_EQ(365, p5.days());
    ASSERT_FALSE(p == p5);
    ASSERT_TRUE(p5 == p5);
}

TEST(Period, Comparison) {
	ASSERT_TRUE(Period::days(11) == Period(PeriodType::DAYS, 11));
	ASSERT_FALSE(Period::days(12) == Period(PeriodType::DAYS, 11));
	ASSERT_FALSE(Period::days(11) == Period(PeriodType::WEEKS, 11));
}

TEST(Period, Output) {
    ASSERT_EQ("NONE", Utils::to_string(Period(PeriodType::NONE, 1)));
    ASSERT_EQ("3M", Utils::to_string(Period(PeriodType::MONTHS, 3)));
    ASSERT_EQ("-1D", Utils::to_string(Period(PeriodType::DAYS, -1)));
	ASSERT_EQ("2Y", Utils::to_string(Period(PeriodType::YEARS, 2)));
	ASSERT_EQ("3W", Utils::to_string(Period(PeriodType::WEEKS, 3)));
}

TEST(Period, FromString) {
	ASSERT_EQ(Period::days(10), Period(std::string("10D").c_str()));
	ASSERT_EQ(Period(), Period(std::string("NONE").c_str()));
	ASSERT_EQ(Period::months(3), Period("3M"));
	ASSERT_EQ(Period::years(-2), Period("-2Y"));
	ASSERT_EQ(Period::weeks(-14), Period("-14W"));
}

TEST(Period, FromTwoDates) {
	Period p(Date(2014, 1, 1), Date(2014, 1, 10));
	ASSERT_EQ(PeriodType::DAYS, p.type);
	ASSERT_EQ(9, p.size);
	p = Period(Date(2014, 1, 1), Date(2014, 1, 1));
	ASSERT_EQ(0, p.size);
	ASSERT_THROW(Period(Date::NAD, Date(2014, 1, 1)), std::domain_error);
	ASSERT_THROW(Period(Date(2014, 1, 1), Date::NAD), std::domain_error);
	ASSERT_THROW(Period(Date::POS_INF, Date(2014, 1, 1)), std::domain_error);
	ASSERT_THROW(Period(Date(2014, 1, 1), Date::POS_INF), std::domain_error);
	ASSERT_THROW(Period(Date::NEG_INF, Date(2014, 1, 1)), std::domain_error);
	ASSERT_THROW(Period(Date(2014, 1, 1), Date::NEG_INF), std::domain_error);
}

TEST(Period, Addition) {
	ASSERT_EQ(Period::months(5), Period::months(2) + Period::months(3));
	ASSERT_EQ(Period::years(5), Period::years(-2) + Period::years(7));
	ASSERT_EQ(Period::days(10), Period::weeks(1) + Period::days(3));
	ASSERT_EQ(Period::days(10), Period::days(3) + Period::weeks(1));
	ASSERT_EQ(Period::months(14), Period::years(1) + Period::months(2));
	ASSERT_EQ(Period::months(14), Period::months(2) + Period::years(1));
	ASSERT_THROW(Period::years(2) + Period::days(3), std::domain_error);
	ASSERT_THROW(Period::years(2) + Period::weeks(3), std::domain_error);
	ASSERT_THROW(Period::months(2) + Period::days(3), std::domain_error);
	ASSERT_THROW(Period::months(2) + Period::weeks(3), std::domain_error);
}

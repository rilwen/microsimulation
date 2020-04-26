// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/dates.hpp"
#include "core/daycount.hpp"
#include "core/period.hpp"
#include <limits>
#include <sstream>

using namespace averisera;

TEST(Daycount, Test) {
    const Date d1(2012, 2, 7);
    const Date d2(2014, 5, 17);
    const Date d3(2013, 5, 17);
    ASSERT_NEAR(2.2739726027, Daycount::DAYS_365()->calc(d1, d2), 1E-9);
    ASSERT_NEAR(2.2724161533, Daycount::DAYS_365_25()->calc(d1, d2), 1E-9);
    ASSERT_NEAR(2.2715098436, Daycount::YEAR_FRACT()->calc(d1, d2), 1E-9);
    ASSERT_NEAR(1.2715098436, Daycount::YEAR_FRACT()->calc(d1, d3), 1E-9);
    ASSERT_NEAR(0, Daycount::DAYS_365()->calc(d1, d1), 1E-9);
    ASSERT_NEAR(0, Daycount::DAYS_365_25()->calc(d1, d1), 1E-9);
    ASSERT_NEAR(0, Daycount::YEAR_FRACT()->calc(d1, d1), 1E-9);
    ASSERT_NEAR(-2.2739726027, Daycount::DAYS_365()->calc(d2, d1), 1E-9);
    ASSERT_NEAR(-2.2724161533, Daycount::DAYS_365_25()->calc(d2, d1), 1E-9);
    ASSERT_NEAR(-2.2715098436, Daycount::YEAR_FRACT()->calc(d2, d1), 1E-9);
    ASSERT_NEAR(-1.2715098436, Daycount::YEAR_FRACT()->calc(d3, d1), 1E-9);
    ASSERT_NEAR(2 / 366.0, Daycount::YEAR_FRACT()->calc(d1, d1 + Period(PeriodType::DAYS, 2)), 1E-14);
    ASSERT_NEAR(2 / 365.0, Daycount::YEAR_FRACT()->calc(d2, d2 + Period(PeriodType::DAYS, 2)), 1E-14);

    ASSERT_EQ(d2, Daycount::DAYS_365()->add_year_fraction(d1, 2.2739726027));
    ASSERT_EQ(d2, Daycount::DAYS_365_25()->add_year_fraction(d1, 2.2724161533));
    ASSERT_EQ(d2, Daycount::YEAR_FRACT()->add_year_fraction(d1, 2.2715098436));
    ASSERT_EQ(d3, Daycount::YEAR_FRACT()->add_year_fraction(d1, 1.2715098436));
    ASSERT_EQ(d1, Daycount::DAYS_365()->add_year_fraction(d2, -2.2739726027));
    ASSERT_EQ(d1, Daycount::DAYS_365_25()->add_year_fraction(d2, -2.2724161533));
    ASSERT_EQ(d1, Daycount::YEAR_FRACT()->add_year_fraction(d2, -2.2715098436));
    ASSERT_EQ(d1, Daycount::YEAR_FRACT()->add_year_fraction(d3, -1.2715098436));
    ASSERT_EQ(d1, Daycount::DAYS_365()->add_year_fraction(d1, 0));
    ASSERT_EQ(d1, Daycount::DAYS_365_25()->add_year_fraction(d1, 0));
    ASSERT_EQ(d1, Daycount::YEAR_FRACT()->add_year_fraction(d1, 0));
    ASSERT_EQ(d1, Daycount::DAYS_365()->add_year_fraction(d1, -0.));
    ASSERT_EQ(d1, Daycount::DAYS_365_25()->add_year_fraction(d1, -0.));
    ASSERT_EQ(d1, Daycount::YEAR_FRACT()->add_year_fraction(d1, -0.));

    std::stringstream ss;
    ss << *Daycount::DAYS_365();
    ASSERT_EQ("DAYS_365", ss.str());
    ss.str("");
    ss << *Daycount::DAYS_365_25();
    ASSERT_EQ("DAYS_365.25", ss.str());
    ss.str("");
    ss << *Daycount::YEAR_FRACT();
    ASSERT_EQ("YEAR_FRACT", ss.str());
}

TEST(Daycount, Days365) {
    const Daycount& dcc = *Daycount::DAYS_365();
    const Date d1(2015, 4, 4);
    Date d2 = dcc.add_year_fraction(d1, std::numeric_limits<double>::infinity());
    ASSERT_TRUE(d2.is_pos_infinity()) << d2;
    ASSERT_THROW(dcc.add_year_fraction(d1, 1E50), std::out_of_range);
    d2 = dcc.add_year_fraction(d1, -std::numeric_limits<double>::infinity());
    ASSERT_TRUE(d2.is_neg_infinity()) << d2;
    ASSERT_THROW(dcc.add_year_fraction(d1, -1E50), std::out_of_range);
    ASSERT_NEAR(615.6630136986302, dcc.calc(Date::MIN, d1), 1E-14);
    ASSERT_NEAR(7990.046575342466, dcc.calc(d1, Date::MAX), 1E-14);
    ASSERT_NEAR(-615.6630136986302, dcc.calc(d1, Date::MIN), 1E-14);
    ASSERT_NEAR(-7990.046575342466, dcc.calc(Date::MAX, d1), 1E-14);
    ASSERT_EQ(Date::MAX, dcc.add_year_fraction(Date::MAX, 0));
    ASSERT_EQ(Date::MIN, dcc.add_year_fraction(Date::MIN, 0));
    ASSERT_EQ(Date::MAX, dcc.add_year_fraction(d1, dcc.calc(d1, Date::MAX)));
    ASSERT_EQ(Date::MIN, dcc.add_year_fraction(d1, dcc.calc(d1, Date::MIN)));
}

TEST(Daycount, YearFract) {
    const Daycount& dcc = *Daycount::YEAR_FRACT();
    const Date d1(2015, 4, 4);
    Date d2 = dcc.add_year_fraction(d1, std::numeric_limits<double>::infinity());
    ASSERT_TRUE(d2.is_pos_infinity()) << d2;
    ASSERT_THROW(dcc.add_year_fraction(d1, 1E50), std::out_of_range);
    d2 = dcc.add_year_fraction(d1, -std::numeric_limits<double>::infinity());
    ASSERT_TRUE(d2.is_neg_infinity()) << d2;
    ASSERT_THROW(dcc.add_year_fraction(d1, -1E50), std::out_of_range);
    ASSERT_NEAR(615.2547945205479452, dcc.calc(Date::MIN, d1), 1E-14);
    ASSERT_NEAR(7984.742465753425, dcc.calc(d1, Date::MAX), 1E-14);
    ASSERT_NEAR(-615.2547945205479452, dcc.calc(d1, Date::MIN), 1E-14);
    ASSERT_NEAR(-7984.742465753425, dcc.calc(Date::MAX, d1), 1E-14);
    ASSERT_EQ(Date::MAX, dcc.add_year_fraction(Date::MAX, 0));
    ASSERT_EQ(Date::MIN, dcc.add_year_fraction(Date::MIN, 0));
    ASSERT_EQ(Date::MAX, dcc.add_year_fraction(d1, dcc.calc(d1, Date::MAX)));
    ASSERT_EQ(Date::MIN, dcc.add_year_fraction(d1, dcc.calc(d1, Date::MIN)));	
}

TEST(Daycount, FromString) {
	ASSERT_EQ(Daycount::YEAR_FRACT(), Daycount::from_string(std::string("YEAR_FRACT").c_str()));
	ASSERT_EQ(Daycount::DAYS_365(), Daycount::from_string("DAYS_365"));
	ASSERT_EQ(Daycount::DAYS_365_25(), Daycount::from_string("DAYS_365.25"));
}

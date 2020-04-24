#include "core/dates.hpp"
#include "core/period.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

using namespace averisera;

// TEST(Dates, AsDouble) {
// 	Date date(2010, 12, 31);
// 	ASSERT_NEAR(2010 + 364.0 / Date::YEAR_FRACTION_BASE, Date::as_double(date), 1E-10);
// }

TEST(Dates, DefaultConstructor) {
	Date date;
	ASSERT_TRUE(date.is_not_a_date());
}

TEST(Dates, ExtremeDates) {
	Date d1(1401, 1, 1);
	Date d2(4500, 6, 1);
}

TEST(Dates, MakeDateIterator) {
    Date start(2015, 3, 1);
    ASSERT_EQ(nullptr, start.make_date_iterator(Period(PeriodType::NONE, 2)));
    auto day_iter_ptr = start.make_date_iterator(Period(PeriodType::DAYS, 2));
    auto& day_iter = *day_iter_ptr;
    ++day_iter;
    ASSERT_EQ(Date(2015, 3, 3), *day_iter);
    ++day_iter;
    ASSERT_EQ(Date(2015, 3, 5), *day_iter);
    --day_iter;
    ASSERT_EQ(Date(2015, 3, 3), *day_iter);
    auto week_iter_ptr = start.make_date_iterator(Period(PeriodType::WEEKS, -1));
    auto& week_iter = *week_iter_ptr;
    ++week_iter;
    ASSERT_EQ(Date(2015, 2, 22), *week_iter);
    auto month_iter_ptr = Date((*week_iter)).make_date_iterator(Period(PeriodType::MONTHS, 3));
    ++(*month_iter_ptr);
    ASSERT_EQ(Date(2015, 5, 22), **month_iter_ptr);
    auto year_iter_ptr = start.make_date_iterator(Period(PeriodType::YEARS, 1));
    ++(*year_iter_ptr);
    ASSERT_EQ(Date(2016, 3, 1), **year_iter_ptr);
}

TEST(Dates, AddPeriod) {
    Date start(2015, 3, 1);
    Date date = start + Period(PeriodType::DAYS, 1);
    ASSERT_EQ(Date(2015, 3, 2), date);
    date = start + Period(PeriodType::WEEKS, 2);
    ASSERT_EQ(Date(2015, 3, 15), date);
    date = start + Period(PeriodType::MONTHS, -2);
    ASSERT_EQ(Date(2015, 1, 1), date);
    date = start + Period(PeriodType::YEARS, 3);
    ASSERT_EQ(Date(2018, 3, 1), date);

    ASSERT_TRUE((Date::NAD + Period::days(10)).is_not_a_date());
    
    ASSERT_THROW(start + Period::years(10000), std::out_of_range);
    ASSERT_THROW(start + Period::months(100000), std::out_of_range);
    ASSERT_THROW(start + Period::weeks(10000*52), std::out_of_range);
    ASSERT_THROW(start + Period::days(10000000), std::out_of_range);    

	ASSERT_EQ(Date(1500, 6, 2), Date(1500, 6, 1) + Period::days(1));
	ASSERT_EQ(Date(1501, 6, 1), Date(1500, 6, 1) + Period::years(1));
	ASSERT_EQ(Date(9500, 6, 2), Date(9500, 6, 1) + Period::days(1));
	ASSERT_EQ(Date(9501, 6, 1), Date(9500, 6, 1) + Period::years(1));
	ASSERT_EQ(Date(3500, 6, 1), Date(1500, 6, 1) + Period::years(2000));

	ASSERT_EQ(Date(1992, 2, 29), Date(1991, 2, 28) + Period::years(1));
	ASSERT_EQ(Date(1992, 2, 29), Date(1991, 2, 28) + Period::months(12));
	ASSERT_EQ(Date(1992, 2, 29), Date(1991, 2, 28) + Period::days(366));
	//ASSERT_EQ(Date(4500, 6, 1), Date(1500, 6, 1) + Period::years(3000));
	/*boost::gregorian::date d1(1500, 6, 1);
	boost::gregorian::date d2(4500, 6, 1);
	const auto duration = boost::gregorian::years(3000);
	std::cout << d1 << " + " << duration.number_of_years() << "Y\n";
	const auto d3 = d1 + duration;
	std::cout << d3 << "\n";
	ASSERT_EQ(d2, d3);*/
}

TEST(Dates, SubtractPeriod) {
    Date start(2015, 3, 1);
    Date date = start - Period(PeriodType::DAYS, -1);
    ASSERT_EQ(Date(2015, 3, 2), date);
    date = start - Period(PeriodType::WEEKS, -2);
    ASSERT_EQ(Date(2015, 3, 15), date);
    date = start - Period(PeriodType::MONTHS, 2);
    ASSERT_EQ(Date(2015, 1, 1), date);
    date = start - Period(PeriodType::YEARS, -3);
    ASSERT_EQ(Date(2018, 3, 1), date);

    ASSERT_TRUE((Date::NAD - Period::days(10)).is_not_a_date());

    ASSERT_THROW(start - Period::years(10000), std::out_of_range);
    ASSERT_THROW(start - Period::months(100000), std::out_of_range);
    ASSERT_THROW(start - Period::weeks(10000*52), std::out_of_range);
    ASSERT_THROW(start - Period::days(10000000), std::out_of_range);
}



// TEST(Dates, AddYearFraction) {
//     Date d(2015, 1, 1);
//     ASSERT_EQ(Date(2016, 1, 1), Date::add_year_fraction(d, 1.));
//     ASSERT_EQ(Date(2014, 1, 1), Date::add_year_fraction(d, -1.));
//     ASSERT_EQ(Date::POS_INF, Date::add_year_fraction(d, std::numeric_limits<double>::infinity()));
//     d = Date(2015, 3, 10);
//     const double yfr = 0.24;
//     Date d2 = Date::add_year_fraction(d, yfr);
//     ASSERT_NEAR(yfr, Date::year_fraction(d, d2), 1.0 / Date::YEAR_FRACTION_BASE);
// }

// TEST(Dates, FromAsDouble) {
//     Date d(2014, 6, 12);
//     double x = Date::as_double(d);
//     ASSERT_EQ(d, Date::from_double(x));
//     ASSERT_NE(d, Date::from_double(x - 1E-12));
//     ASSERT_EQ(d, Date::from_double(x + 1E-12));
// }

TEST(Dates, IsLeap) {
    ASSERT_TRUE(Date::is_leap(2012));
    ASSERT_FALSE(Date::is_leap(2011));
    ASSERT_FALSE(Date::is_leap(1900));
    ASSERT_TRUE(Date::is_leap(2000));
}

TEST(Dates, FromISOString) {
	ASSERT_EQ(Date(1989, 6, 4), Date::from_iso_string("19890604"));
	ASSERT_EQ(Date(1989, 6, 4), Date::from_iso_string(std::string("19890604")));
}

TEST(Dates, FromString) {
	ASSERT_EQ(Date(1989, 6, 4), Date::from_string(std::string("1989/06/04")));
	ASSERT_EQ(Date(1989, 6, 4), Date::from_string(std::string("1989-06-04")));
	ASSERT_EQ(Date(1989, 6, 4), Date::from_string(std::string("1989-Jun-04")));
	ASSERT_EQ(Date(1989, 6, 4), Date::from_string(std::string("1989-June-04")));
	ASSERT_EQ(Date(1989, 6, 4), Date::from_string("1989/06/04"));
	ASSERT_EQ(Date(1989, 6, 4), Date::from_string("1989-06-04"));
	ASSERT_EQ(Date(1989, 6, 4), Date::from_string("1989-Jun-04"));
	ASSERT_EQ(Date(1989, 6, 4), Date::from_string("1989-June-04"));
}

TEST(Dates, Print) {
	Date d(1989, 6, 4);
	std::stringstream ss;
	ss << d;
	ASSERT_EQ("1989-06-04", ss.str()) << ss.str();
}

TEST(Dates, DayOfYear) {
	Date d(2010, 1, 11);
	ASSERT_EQ(11, d.day_of_year());
}

TEST(Dates, Dist) {
	Date d1(1990, 3, 15);
	Date d2(1990, 3, 20);
	ASSERT_EQ(5, d1.dist(d2, Period::days(1)));
	ASSERT_EQ(-5, d2.dist(d1, Period::days(1)));
	ASSERT_EQ(0, d1.dist(d1, Period::days(1)));
	ASSERT_EQ(2, d1.dist(d2, Period::days(2)));
	ASSERT_EQ(-2, d2.dist(d1, Period::days(2)));
	ASSERT_EQ(0, d1.dist(d2, Period::weeks(1)));
	d2 = Date(1991, 3, 15);
	ASSERT_EQ(12, d1.dist(d2, Period::months(1)));
	ASSERT_EQ(4, d1.dist(d2, Period::months(3)));
	ASSERT_EQ(-4, d2.dist(d1, Period::months(3)));
	ASSERT_EQ(1, d1.dist(d2, Period::years(1)));
	ASSERT_EQ(0, d1.dist(d2, Period::years(2)));
	ASSERT_EQ(52, d1.dist(d2, Period::weeks(1)));
	ASSERT_EQ(26, d1.dist(d2, Period::weeks(2)));
	ASSERT_EQ(-52, d2.dist(d1, Period::weeks(1)));
	ASSERT_EQ(-26, d2.dist(d1, Period::weeks(2)));
	d2 = Date(1991, 3, 16);
	ASSERT_EQ(12, d1.dist(d2, Period::months(1)));
	ASSERT_EQ(4, d1.dist(d2, Period::months(3)));
	ASSERT_EQ(-4, d2.dist(d1, Period::months(3)));
	ASSERT_EQ(1, d1.dist(d2, Period::years(1)));
	ASSERT_EQ(0, d1.dist(d2, Period::years(2)));
	ASSERT_EQ(52, d1.dist(d2, Period::weeks(1)));
	ASSERT_EQ(26, d1.dist(d2, Period::weeks(2)));
	ASSERT_EQ(-52, d2.dist(d1, Period::weeks(1)));
	ASSERT_EQ(-26, d2.dist(d1, Period::weeks(2)));
	d2 = Date(1991, 3, 10);
	ASSERT_EQ(11, d1.dist(d2, Period::months(1)));
	ASSERT_EQ(3, d1.dist(d2, Period::months(3)));
	ASSERT_EQ(-3, d2.dist(d1, Period::months(3)));
	ASSERT_EQ(0, d1.dist(d2, Period::years(1)));
	ASSERT_EQ(0, d1.dist(d2, Period::years(2)));
	ASSERT_EQ(51, d1.dist(d2, Period::weeks(1)));
	ASSERT_EQ(25, d1.dist(d2, Period::weeks(2)));
	ASSERT_EQ(-51, d2.dist(d1, Period::weeks(1)));
	ASSERT_EQ(-25, d2.dist(d1, Period::weeks(2)));
	d1 = Date(2000, 2, 29);
	d2 = Date(2001, 2, 28);
	ASSERT_EQ(1, d1.dist(d2, Period::years(1)));
	ASSERT_EQ(12, d1.dist(d2, Period::months(1)));
}

TEST(Dates, DistDays) {
	ASSERT_EQ(1, Date(1991, 3, 12).dist_days(Date(1991, 3, 13)));
	ASSERT_EQ(-1, Date(1991, 3, 13).dist_days(Date(1991, 3, 12)));
	ASSERT_EQ(366, Date(1991, 3, 12).dist_days(Date(1992, 3, 12)));
	ASSERT_EQ(366, Date(1992, 2, 1).dist_days(Date(1993, 2, 1)));
}

TEST(Dates, DistYears) {
	ASSERT_EQ(0, Date(1991, 3, 12).dist_years(Date(1991, 3, 13)));
	ASSERT_EQ(0, Date(1991, 3, 12).dist_years(Date(1992, 3, 11)));
	ASSERT_EQ(1, Date(1991, 3, 12).dist_years(Date(1992, 3, 13)));
	ASSERT_EQ(1, Date(1991, 3, 12).dist_years(Date(1992, 6, 13)));
	ASSERT_EQ(4, Date(1992, 2, 29).dist_years(Date(1996, 2, 29)));
	ASSERT_EQ(3, Date(1992, 2, 29).dist_years(Date(1995, 2, 28)));
	ASSERT_EQ(2, Date(1992, 2, 29).dist_years(Date(1995, 2, 27)));
	ASSERT_EQ(0, Date(1991, 2, 28).dist_years(Date(1992, 2, 28)));
	ASSERT_EQ(1, Date(1991, 2, 28).dist_years(Date(1992, 2, 29)));
	ASSERT_EQ(-1, Date(1991, 5, 1).dist_years(Date(1990, 4, 30)));
	ASSERT_EQ(20, Date(1995, 5, 1).dist_years(Date(2015, 5, 1)));
	ASSERT_EQ(2000, Date(1800, 5, 1).dist_years(Date(3800, 5, 1)));
	ASSERT_EQ(2000, Date(1500, 5, 1).dist_years(Date(3500, 5, 2)));
	ASSERT_EQ(1999, Date(1500, 5, 1).dist_years(Date(3500, 4, 30)));
	ASSERT_EQ(1999, Date(1500, 5, 1).dist_years(Date(3500, 1, 1)));
	ASSERT_EQ(2000, Date(1500, 5, 1).dist_years(Date(3500, 10, 1)));
}

TEST(Dates, YMD) {
	const Date d(1991, 7, 1);
	const Date::ymd_type ymd = d.year_month_day();
	ASSERT_EQ(1991, ymd.year);
	ASSERT_EQ(7, ymd.month);
	ASSERT_EQ(1, ymd.day);
}

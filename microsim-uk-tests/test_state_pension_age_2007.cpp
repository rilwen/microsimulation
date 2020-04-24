/*
(C) Averisera Ltd 2017
*/
#include <gtest/gtest.h>
#include "microsim-uk/state_pension_age_2007.hpp"
#include "core/daycount.hpp"
#include "core/period.hpp"
#include "spa_tests.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(StatePensionAge2007, Female) {
	ASSERT_EQ(Date(1980, 6, 15), StatePensionAge2007::get_date_spa_reached_female(Date(1920, 6, 15)));
	ASSERT_EQ(Date(1992, 6, 15), StatePensionAge2007::get_date_spa_reached_female(Date(1932, 6, 15)));
	ASSERT_EQ(Date(2010, 4, 5), StatePensionAge2007::get_date_spa_reached_female(Date(1950, 4, 5)));
	ASSERT_EQ(Date(2010, 5, 6), StatePensionAge2007::get_date_spa_reached_female(Date(1950, 4, 6)));
	ASSERT_EQ(Date(2010, 5, 6), StatePensionAge2007::get_date_spa_reached_female(Date(1950, 4, 20)));
	ASSERT_EQ(Date(2010, 7, 6), StatePensionAge2007::get_date_spa_reached_female(Date(1950, 5, 6)));
	ASSERT_EQ(Date(2020, 3, 6), StatePensionAge2007::get_date_spa_reached_female(Date(1955, 4, 5)));
	ASSERT_EQ(Date(2020, 4, 6), StatePensionAge2007::get_date_spa_reached_female(Date(1955, 4, 6)));
	ASSERT_EQ(Date(2022, 4, 10), StatePensionAge2007::get_date_spa_reached_female(Date(1957, 4, 10)));
	ASSERT_EQ(Date(2024, 4, 5), StatePensionAge2007::get_date_spa_reached_female(Date(1959, 4, 5)));
	const Date start(1959, 4, 6);
	const Date end(2041, 4, 6);
	const Period delta(PeriodType::DAYS, 1);
	for (Date d = start; d < end; d = d + delta) {
		ASSERT_EQ(StatePensionAge2007::get_date_spa_reached_male(d), StatePensionAge2007::get_date_spa_reached_female(d)) << d;
	}
	for (Date d = Date(1920, 1, 1); d < end; d = d + delta) {
		ASSERT_EQ(StatePensionAge2007::get_date_spa_reached(Sex::FEMALE, d), StatePensionAge2007::get_date_spa_reached_female(d)) << d;
	}
}

TEST(StatePensionAge2007, Male) {
	ASSERT_EQ(Date(1985, 6, 15), StatePensionAge2007::get_date_spa_reached_male(Date(1920, 6, 15)));
	ASSERT_EQ(Date(2020, 4, 6), StatePensionAge2007::get_date_spa_reached_male(Date(1955, 4, 6)));
	ASSERT_EQ(Date(2024, 4, 5), StatePensionAge2007::get_date_spa_reached_male(Date(1959, 4, 5)));
	ASSERT_EQ(Date(2024, 5, 6), StatePensionAge2007::get_date_spa_reached_male(Date(1959, 4, 6)));
	ASSERT_EQ(Date(2024, 5, 6), StatePensionAge2007::get_date_spa_reached_male(Date(1959, 5, 5)));
    ASSERT_EQ(Date(2026, 3, 6), StatePensionAge2007::get_date_spa_reached_male(Date(1960, 4, 5)));

    ASSERT_EQ(Date(2030, 7, 20), StatePensionAge2007::get_date_spa_reached_male(Date(1964, 7, 20)));

	ASSERT_EQ(Date(2034, 5, 6), StatePensionAge2007::get_date_spa_reached_male(Date(1968, 4, 6)));
	ASSERT_EQ(Date(2034, 5, 6), StatePensionAge2007::get_date_spa_reached_male(Date(1968, 5, 5)));
    ASSERT_EQ(Date(2036, 3, 6), StatePensionAge2007::get_date_spa_reached_male(Date(1969, 4, 4)));
    ASSERT_EQ(Date(2036, 3, 6), StatePensionAge2007::get_date_spa_reached_male(Date(1969, 4, 5)));

    ASSERT_EQ(Date(2040, 7, 20), StatePensionAge2007::get_date_spa_reached_male(Date(1973, 7, 20)));

	ASSERT_EQ(Date(2044, 5, 6), StatePensionAge2007::get_date_spa_reached_male(Date(1977, 4, 6)));
	ASSERT_EQ(Date(2044, 5, 6), StatePensionAge2007::get_date_spa_reached_male(Date(1977, 5, 5)));
    ASSERT_EQ(Date(2046, 3, 6), StatePensionAge2007::get_date_spa_reached_male(Date(1978, 4, 4)));
    ASSERT_EQ(Date(2046, 3, 6), StatePensionAge2007::get_date_spa_reached_male(Date(1978, 4, 5)));

	ASSERT_EQ(Date(2046, 4, 6), StatePensionAge2007::get_date_spa_reached_male(Date(1978, 4, 6)));
	ASSERT_EQ(Date(2046, 7, 3), StatePensionAge2007::get_date_spa_reached_male(Date(1978, 7, 3)));

	const Date end(2041, 4, 6);
	const Period delta(PeriodType::DAYS, 1);
	for (Date d = Date(1920, 1, 1); d < end; d = d + delta) {
		ASSERT_EQ(StatePensionAge2007::get_date_spa_reached(Sex::MALE, d), StatePensionAge2007::get_date_spa_reached_male(d)) << d;
	}
}

//TEST(StatePensionAge2007, ChartDOB) {
//	const auto dc = Daycount::YEAR_FRACT();
//	std::cout << "DATE\tMALE\tFEMALE\n";
//	for (Date::year_type yr = 1920; yr <= 2020; ++yr) {
//		for (Date::month_type mnth = 1; mnth <= 12; ++mnth) {
//			const Date dob(yr, mnth, 1);
//			const Date spa_m = StatePensionAge2007::get_date_spa_reached_male(dob);
//			const Date spa_f = StatePensionAge2007::get_date_spa_reached_female(dob);
//			std::cout << dob << "\t" << dc->calc(dob, spa_m) << "\t" << dc->calc(dob, spa_f) << "\n";
//		}
//	}
//}

//TEST(StatePensionAge2007, ChartSimDate) {
//	const Date start(1991, 7, 1);
//	const Date end(2061, 7, 1);
//	const auto spa_at_ret_male = averisera::testing::make_spa_at_retirement_date_series<StatePensionAge2007>(start, end, Sex::MALE);
//	const auto spa_at_ret_female = averisera::testing::make_spa_at_retirement_date_series<StatePensionAge2007>(start, end, Sex::FEMALE);
//	static const Period delta(PeriodType::DAYS, 1);
//	std::cout << "DATE,MALE,FEMALE\n";
//	Date date = start;
//	while (date <= end) {
//		std::cout << date << "," << *(spa_at_ret_male.last_value(date)) << "," << *(spa_at_ret_female.last_value(date)) << "\n";
//		date = date + delta;
//	}
//}

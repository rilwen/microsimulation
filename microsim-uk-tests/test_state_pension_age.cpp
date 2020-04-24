/*
(C) Averisera Ltd 2017
*/
#include <gtest/gtest.h>
#include "microsim-uk/state_pension_age.hpp"
#include "core/daycount.hpp"
#include "core/period.hpp"
#include "spa_tests.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(StatePensionAge, Female) {
	ASSERT_EQ(Date(1980, 6, 15), StatePensionAge::get_date_spa_reached_female(Date(1920, 6, 15)));
	ASSERT_EQ(Date(1992, 6, 15), StatePensionAge::get_date_spa_reached_female(Date(1932, 6, 15)));
	ASSERT_EQ(Date(2010, 4, 5), StatePensionAge::get_date_spa_reached_female(Date(1950, 4, 5)));
	ASSERT_EQ(Date(2010, 5, 6), StatePensionAge::get_date_spa_reached_female(Date(1950, 4, 6)));
	ASSERT_EQ(Date(2010, 5, 6), StatePensionAge::get_date_spa_reached_female(Date(1950, 4, 20)));
	ASSERT_EQ(Date(2010, 7, 6), StatePensionAge::get_date_spa_reached_female(Date(1950, 5, 6)));
	ASSERT_EQ(Date(2016, 7, 6), StatePensionAge::get_date_spa_reached_female(Date(1953, 4, 6)));
	ASSERT_EQ(Date(2018, 11, 6), StatePensionAge::get_date_spa_reached_female(Date(1953, 12, 5)));
	ASSERT_EQ(Date(2020, 9, 6), StatePensionAge::get_date_spa_reached_female(Date(1954, 10, 5)));
	ASSERT_EQ(Date(2020, 10, 6), StatePensionAge::get_date_spa_reached_female(Date(1954, 10, 6)));
	ASSERT_EQ(Date(2023, 4, 10), StatePensionAge::get_date_spa_reached_female(Date(1957, 4, 10)));
	ASSERT_EQ(Date(2025, 4, 5), StatePensionAge::get_date_spa_reached_female(Date(1959, 4, 5)));
	const Date start(1959, 4, 6);
	const Date end(2041, 4, 6);
	const Period delta(PeriodType::DAYS, 1);
	for (Date d = start; d < end; d = d + delta) {
		ASSERT_EQ(StatePensionAge::get_date_spa_reached_male(d), StatePensionAge::get_date_spa_reached_female(d)) << d;
	}
	for (Date d = Date(1920, 1, 1); d < end; d = d + delta) {
		ASSERT_EQ(StatePensionAge::get_date_spa_reached(Sex::FEMALE, d), StatePensionAge::get_date_spa_reached_female(d)) << d;
	}
}

TEST(StatePensionAge, Male) {
	ASSERT_EQ(Date(1985, 6, 15), StatePensionAge::get_date_spa_reached_male(Date(1920, 6, 15)));
	ASSERT_EQ(Date(2019, 3, 6), StatePensionAge::get_date_spa_reached_male(Date(1953, 12, 6)));
	ASSERT_EQ(Date(2020, 9, 6), StatePensionAge::get_date_spa_reached_female(Date(1954, 10, 5)));
	ASSERT_EQ(Date(2021, 4, 6), StatePensionAge::get_date_spa_reached_male(Date(1955, 4, 6)));
	ASSERT_EQ(Date(2025, 4, 5), StatePensionAge::get_date_spa_reached_male(Date(1959, 4, 5)));
	ASSERT_EQ(Date(2025, 4, 6), StatePensionAge::get_date_spa_reached_male(Date(1959, 4, 6)));
	ASSERT_EQ(Date(2025, 5, 5), StatePensionAge::get_date_spa_reached_male(Date(1959, 5, 5)));
    ASSERT_EQ(Date(2026, 4, 5), StatePensionAge::get_date_spa_reached_male(Date(1960, 4, 5)));

    ASSERT_EQ(Date(2031, 7, 20), StatePensionAge::get_date_spa_reached_male(Date(1964, 7, 20)));

	ASSERT_EQ(Date(2035, 4, 6), StatePensionAge::get_date_spa_reached_male(Date(1968, 4, 6)));
	ASSERT_EQ(Date(2035, 5, 5), StatePensionAge::get_date_spa_reached_male(Date(1968, 5, 5)));
    ASSERT_EQ(Date(2036, 4, 4), StatePensionAge::get_date_spa_reached_male(Date(1969, 4, 4)));
    ASSERT_EQ(Date(2036, 4, 5), StatePensionAge::get_date_spa_reached_male(Date(1969, 4, 5)));

    ASSERT_EQ(Date(2040, 7, 20), StatePensionAge::get_date_spa_reached_male(Date(1973, 7, 20)));

	ASSERT_EQ(Date(2044, 5, 6), StatePensionAge::get_date_spa_reached_male(Date(1977, 4, 6)));
	ASSERT_EQ(Date(2044, 5, 6), StatePensionAge::get_date_spa_reached_male(Date(1977, 5, 5)));
    ASSERT_EQ(Date(2046, 3, 6), StatePensionAge::get_date_spa_reached_male(Date(1978, 4, 4)));
    ASSERT_EQ(Date(2046, 3, 6), StatePensionAge::get_date_spa_reached_male(Date(1978, 4, 5)));

	ASSERT_EQ(Date(2046, 4, 6), StatePensionAge::get_date_spa_reached_male(Date(1978, 4, 6)));
	ASSERT_EQ(Date(2046, 7, 3), StatePensionAge::get_date_spa_reached_male(Date(1978, 7, 3)));

	const Date end(2041, 4, 6);
	const Period delta(PeriodType::DAYS, 1);
	for (Date d = Date(1920, 1, 1); d < end; d = d + delta) {
		ASSERT_EQ(StatePensionAge::get_date_spa_reached(Sex::MALE, d), StatePensionAge::get_date_spa_reached_male(d)) << d;
	}
}

//TEST(StatePensionAge, ChartDOB) {
//	const auto dc = Daycount::YEAR_FRACT();
//	std::cout << "DATE\tMALE\tFEMALE\n";
//	for (Date::year_type yr = 1920; yr <= 2020; ++yr) {
//		for (Date::month_type mnth = 1; mnth <= 12; ++mnth) {
//			const Date dob(yr, mnth, 1);
//			const Date spa_m = StatePensionAge::get_date_spa_reached_male(dob);
//			const Date spa_f = StatePensionAge::get_date_spa_reached_female(dob);
//			std::cout << dob << "\t" << dc->calc(dob, spa_m) << "\t" << dc->calc(dob, spa_f) << "\n";
//		}
//	}
//}

//TEST(StatePensionAge, ChartSimDate) {
//	const Date start(1991, 7, 1);
//	const Date end(2061, 7, 1);
//	const auto spa_at_ret_male = averisera::testing::make_spa_at_retirement_date_series<StatePensionAge>(start, end, Sex::MALE);
//	const auto spa_at_ret_female = averisera::testing::make_spa_at_retirement_date_series<StatePensionAge>(start, end, Sex::FEMALE);
//	static const Period delta(PeriodType::DAYS, 1);
//	std::cout << "DATE,MALE,FEMALE\n";
//	Date date = start;
//	while (date <= end) {
//		std::cout << date << "," << *(spa_at_ret_male.last_value(date)) << "," << *(spa_at_ret_female.last_value(date)) << "\n";
//		date = date + delta;
//	}
//}

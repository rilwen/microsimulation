/*
 * (C) Averisera Ltd 2015
 */
#include <gtest/gtest.h>
#include "microsim-core/schedule.hpp"
#include "microsim-core/schedule_definition.hpp"
#include "core/daycount.hpp"

using namespace averisera;
using namespace averisera::microsim;

static const auto DAYCOUNT = Daycount::DAYS_365();


TEST(Schedule, FromDefinition) {
    ScheduleDefinition def;
    def.start = Date(2013, 11, 30);
    def.end = Date(2014, 12, 10);
	def.frequency.type = PeriodType::MONTHS;
	def.frequency.size = 3;
    def.daycount = DAYCOUNT;
    Schedule schedule(def);
    ASSERT_EQ(def.start, schedule.start_date());
    ASSERT_EQ(def.end, schedule.end_date());
    ASSERT_EQ(4u, schedule.size());
    const Date d2(2014, 2, 28);
    const Date d3(2014, 5, 31);
    const Date d4(2014, 8, 31);
    std::vector<Schedule::period_t> periods(4);
    periods[0] = Schedule::period_t(def.start, d2, DAYCOUNT);
    periods[1] = Schedule::period_t(d2, d3, *DAYCOUNT);
    periods[2] = Schedule::period_t(d3, d4, DAYCOUNT);
    periods[3] = Schedule::period_t(d4, def.end, DAYCOUNT);    
    auto iter = schedule.begin();
    for (unsigned int i = 0; i < 4; ++i) {
        ASSERT_EQ(periods[i], schedule[i]) << i;
        ASSERT_EQ(periods[i], *iter) << i;
        ASSERT_EQ(periods[i].begin, schedule.date(i)) << i;
        ++iter;
    }
    ASSERT_EQ(schedule.end(), iter);
    ASSERT_EQ(def.end, schedule.date(4));
    for (unsigned int i = 0; i < schedule.nbr_dates(); ++i) {
		const Date date = schedule.date(i);
        ASSERT_TRUE(schedule.contains(date)) << i;
        ASSERT_EQ(i, schedule.index(date)) << i;
		if (i > 0) {
			const Date prev_date = schedule.date(i - 1);
			const Date middle = prev_date + (date - prev_date) / 2;
			ASSERT_EQ(i - 1, schedule.find_containing_period(prev_date)) << prev_date;
			ASSERT_EQ(i - 1, schedule.find_containing_period(middle)) << middle;
		}
    }
	ASSERT_THROW(schedule.find_containing_period(schedule.end_date()), std::out_of_range);
    ASSERT_FALSE(schedule.contains(Date(1934, 4, 17)));
    ASSERT_THROW(schedule.index(Date(1939, 9, 1)), std::out_of_range);
}

TEST(Schedule, FromDates) {
    Schedule schedule1({Date(2012, 1, 1), Date(2012, 2, 15), Date(2012, 5, 10)});
    ASSERT_EQ(2u, schedule1.size());
    ASSERT_EQ(Date(2012, 1, 1), schedule1.date(0));
    ASSERT_EQ(Date(2012, 2, 15), schedule1.date(1));
    ASSERT_EQ(Date(2012, 5, 10), schedule1.date(2));
    Schedule schedule2({Date(2012, 1, 1)});
    ASSERT_EQ(1u, schedule2.size());
    ASSERT_EQ(Date(2012,1,1), schedule2.start_date());
    ASSERT_EQ(Date(2012,1,1), schedule2.end_date());
}

TEST(Schedule, Period) {
    SchedulePeriod p;
    ASSERT_EQ(0, p.days);
    ASSERT_EQ(0, p.fract);
    ASSERT_TRUE(p.begin.is_not_a_date());
    ASSERT_TRUE(p.end.is_not_a_date());
    
    const Date b(2014, 4, 1);
    const Date e(2014, 4, 20);
    SchedulePeriod p2(b, e, DAYCOUNT);
    ASSERT_EQ(b, p2.begin);
    ASSERT_EQ(e, p2.end);
    ASSERT_EQ(19, p2.days);
    ASSERT_NEAR(19/365.0, p2.fract, 1E-14);
}

TEST(Schedule, Default) {
    Schedule s;
    ASSERT_TRUE(s.empty());
    ASSERT_EQ(0u, s.size());
    ASSERT_EQ(0u, s.nbr_dates());
    ASSERT_FALSE(s.contains(Date(1934, 4, 17)));
    ASSERT_THROW(s.index(Date(1939, 9, 1)), std::out_of_range);
    ASSERT_TRUE(s.contains(s));
}

TEST(Schedule, ContainsOther) {
    Schedule s;
    Schedule s1({Date(2012, 1, 1), Date(2012, 2, 15), Date(2012, 5, 10)});
    ASSERT_TRUE(s1.contains(s));
    ASSERT_FALSE(s.contains(s1));
    ASSERT_TRUE(s1.contains(s1));
    Schedule s2({Date(2012, 1, 1), Date(2012, 2, 15)});
    ASSERT_TRUE(s1.contains(s2));
    ASSERT_FALSE(s2.contains(s1));
    Schedule s3({Date(2011, 6, 1), Date(2012, 1, 1), Date(2012, 2, 15)});
    ASSERT_TRUE(s3.contains(s2));
    ASSERT_FALSE(s3.contains(s1));
    ASSERT_FALSE(s1.contains(s3));
}

TEST(Schedule, GetYears) {
	ScheduleDefinition def;
	def.start = Date(2013, 11, 30);
	def.end = Date(2015, 3, 10);
	def.frequency.type = PeriodType::MONTHS;
	def.frequency.size = 3;
	def.daycount = DAYCOUNT;
	Schedule schedule(def);
	const std::vector<int> years1(schedule.get_years<int>());
	ASSERT_EQ(std::vector<int>({ 2013, 2014, 2015 }), years1);
	std::vector<int> years2;
	schedule.get_years(years2);
	ASSERT_EQ(std::vector<int>({ 2013, 2014, 2015 }), years2);
}

TEST(Schedule, ExtendBack) {
	const std::vector<int> years({ 2012, 2014, 2016 });
	const std::vector<int> new_years(Schedule::extend_back(years, 4));
	ASSERT_EQ(std::vector<int>({2008, 2009, 2010, 2011, 2012, 2014, 2016}), new_years);
}

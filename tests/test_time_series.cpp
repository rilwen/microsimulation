/*
(C) Averisera Ltd 2015
*/
#include <gtest/gtest.h>
#include "core/time_series.hpp"
#include "core/dates.hpp"
#include <Eigen/Core>
#include <algorithm>
#include <limits>
#include <sstream>

using namespace averisera;

typedef TimeSeries<Date, double> time_series;

TEST(TimeSeries, DefaultConstructor) {
    time_series ts;
    ASSERT_EQ(0u, ts.size());
    ASSERT_EQ(0u, ts.size());
    ASSERT_TRUE(ts.empty());
    ASSERT_THROW(ts.last_value(), std::logic_error);
    ASSERT_THROW(ts.last_time(), std::logic_error);
    ASSERT_THROW(ts.last(), std::logic_error);
    ASSERT_THROW(ts.first_value(), std::logic_error);
    ASSERT_THROW(ts.first_time(), std::logic_error);
    ASSERT_THROW(ts.first(), std::logic_error);
}

TEST(TimeSeries, OnePoint) {
    const std::vector<double> values = { 0. };
    const std::vector<double> times = { 2. };
    TimeSeries<double, double> ts(times, values);
    ASSERT_EQ(1u, ts.size());
    ASSERT_FALSE(ts.empty());
    ASSERT_EQ(values[0], ts.first_value());
    ASSERT_EQ(times[0], ts.first_time());
    ASSERT_EQ(std::make_pair(times[0], values[0]), ts.first());
    ASSERT_EQ(values[0], ts.last_value());
    ASSERT_EQ(times[0], ts.last_time());
    ASSERT_EQ(std::make_pair(times[0], values[0]), ts.last());
}

TEST(TimeSeries, OnePointMove) {
	Eigen::MatrixXd v(Eigen::MatrixXd::Identity(2, 2));
	TimeSeries<double, Eigen::MatrixXd> ts(0.5, std::move(v));
	ASSERT_EQ(1u, ts.size());
	ASSERT_EQ(0.5, ts.first_time());
	ASSERT_EQ(1.0, ts.first_value()(0, 0));
	ASSERT_EQ(1.0, ts.first_value()(1, 1));
	ASSERT_EQ(0.0, ts.first_value()(0, 1));
	ASSERT_EQ(0.0, ts.first_value()(1, 0));
}

TEST(TimeSeries, OnePointScalars) {
    const double value = 0.;
    const double time = 2.;
    TimeSeries<double, double> ts(time, value);
    ASSERT_EQ(1u, ts.size());
    ASSERT_FALSE(ts.empty());
    ASSERT_EQ(value, ts.first_value());
    ASSERT_EQ(time, ts.first_time());
    ASSERT_EQ(std::make_pair(time, value), ts.first());
    ASSERT_EQ(value, ts.last_value());
    ASSERT_EQ(time, ts.last_time());
    ASSERT_EQ(std::make_pair(time, value), ts.last());
}

TEST(TimeSeries, DuplicateTimes) {
	const std::vector<double> times({ 0.1, 0.1 });
	const std::vector<double> values(2);
	typedef TimeSeries<double, double> ts_t; // googletest doesn't like templates inside its macros sometimes
	ASSERT_THROW(ts_t(times, values), std::domain_error);
}

TEST(TimeSeries, SubrangeCopyConstructor) {
    const std::vector<double> values = { 0.1, 0.2, 0.3, 0.4 };
    const Date d1(2010, 1, 1);
    const Date d2(2010, 1, 2);
    const Date d3(2010, 1, 3);
    const Date d4(2010, 1, 4);
    std::vector<Date> times = { d1, d2, d3, d4 };
    time_series ts1(times, values);
    time_series ts2(ts1, 1, 3);
    ASSERT_EQ(2u, ts2.size());
    for (unsigned int i = 0; i < 2; ++i) {
        ASSERT_EQ(ts1[i + 1], ts2[i]) << i;
    }
    ASSERT_THROW(time_series(ts1, 1, 0), std::domain_error);
    ASSERT_THROW(time_series(ts1, 1, 10), std::domain_error);
    ASSERT_THROW(time_series(ts1, 20, 22), std::domain_error);
}

TEST(TimeSeries, VectorConstructorCopy) {
    const std::vector<double> values = {0.1, 0.2};
    const Date d1(2010, 1, 1);
    const Date d2(2010, 1, 2);
    const Date d; // not-a-time
    std::vector<Date> times = {d1, d2};
    time_series ts(times, values);
    ASSERT_EQ(2u, ts.size());
    ASSERT_FALSE(ts.empty());
    for (unsigned int i = 0; i < 2; ++i) {
        ASSERT_EQ(std::make_pair(times[i], values[i]), ts[i]) << i;
    }
    ASSERT_EQ(0.2, ts.last_value());	
    ASSERT_EQ(d2, ts.last_time());
    ASSERT_EQ(std::make_pair(d2, 0.2), ts.last());
    ASSERT_EQ(0.1, ts.first_value());
    ASSERT_EQ(d1, ts.first_time());
    ASSERT_EQ(std::make_pair(d1, 0.1), ts.first());
    std::swap(times[0], times[1]);
    ASSERT_THROW(time_series(times, values), std::domain_error);
    times[0] = times[1];
    ASSERT_THROW(time_series(times, values), std::domain_error);
    times[0] = d;
    ASSERT_THROW(time_series(times, values), std::domain_error);
    times[0] = d1;
    times[1] = d;
    ASSERT_THROW(time_series(times, values), std::domain_error);
}

TEST(TimeSeries, PushBack) {
    ASSERT_THROW(time_series().push_back(Date(), 0.1), std::domain_error);
    time_series ts;
	ASSERT_EQ(0u, ts.size());
    ts.push_back(Date(2011, 4, 12), 0.2);
    ASSERT_EQ(1u, ts.size());
    ASSERT_EQ(Date(2011, 4, 12), ts[0].first);
    ASSERT_EQ(0.2, ts[0].second);
    ASSERT_THROW(ts.push_back(Date(2010, 1, 1), 0.1), std::domain_error);
    ASSERT_THROW(ts.push_back(Date(), 0.1), std::domain_error);
}

TEST(TimeSeries, Value) {
    const std::vector<double> values = {0.1, 0.2};
    const Date d1(2010, 1, 1);
    const Date d2(2010, 1, 4);
    std::vector<Date> times = {d1, d2};
    time_series ts(times, values);
    ASSERT_EQ(nullptr, ts.value(Date(2009, 12, 31)));
    ASSERT_EQ(nullptr, ts.value(Date(2010, 1, 2)));
    ASSERT_EQ(nullptr, ts.value(Date(2010, 1, 5)));
    ASSERT_EQ(values[0], *ts.value(d1));
    ASSERT_EQ(values[1], *ts.value(d2));
}

TEST(TimeSeries, ValueConst) {
	const std::vector<double> values = { 0.1, 0.2 };
	const Date d1(2010, 1, 1);
	const Date d2(2010, 1, 4);
	const std::vector<Date> times({ d1, d2 });
	time_series ts(times, values);
	ASSERT_EQ(nullptr, ts.value(Date(2009, 12, 31)));
	ASSERT_EQ(nullptr, ts.value(Date(2010, 1, 2)));
	ASSERT_EQ(nullptr, ts.value(Date(2010, 1, 5)));
	ASSERT_EQ(values[0], *ts.value(d1));
	ASSERT_EQ(values[1], *ts.value(d2));
}

TEST(TimeSeries, ValueAtIndex) {
	const std::vector<double> values = { 0.1, 0.2 };
	const Date d1(2010, 1, 1);
	const Date d2(2010, 1, 4);
	std::vector<Date> times = { d1, d2 };
	time_series ts(times, values);
	ts.value_at_index(1) = 0.3;
	ASSERT_EQ(0.3, ts[1].second);
}

TEST(TimeSeries, LastValue) {
    const std::vector<double> values = {0.1, 0.2};
    const Date d1(2010, 1, 1);
    const Date d2(2010, 1, 4);
    std::vector<Date> times = {d1, d2};
    time_series ts(times, values);
    ASSERT_EQ(nullptr, ts.last_value(Date(2009, 12, 31)));
    ASSERT_EQ(values[0], *ts.last_value(Date(2010, 1, 2)));
    ASSERT_EQ(values[1], *ts.last_value(Date(2010, 1, 5)));
    ASSERT_EQ(values[0], *ts.last_value(d1));
    ASSERT_EQ(values[1], *ts.last_value(d2));
}

TEST(TimeSeries, LastTime) {
    const std::vector<double> values = {0.1, 0.2};
    const Date d1(2010, 1, 1);
    const Date d2(2010, 1, 4);
    std::vector<Date> times = {d1, d2};
    time_series ts(times, values);
    ASSERT_EQ(nullptr, ts.last_time(Date(2009, 12, 31)));
    ASSERT_EQ(times[0], *ts.last_time(Date(2010, 1, 2)));
    ASSERT_EQ(times[1], *ts.last_time(Date(2010, 1, 5)));
    ASSERT_EQ(times[0], *ts.last_time(d1));
    ASSERT_EQ(times[1], *ts.last_time(d2));
}

TEST(TimeSeries, LastIndex) {
    const std::vector<double> values = { 0.1, 0.2 };
    const Date d1(2010, 1, 1);
    const Date d2(2010, 1, 4);
    std::vector<Date> times = { d1, d2 };
    time_series ts(times, values);
    ASSERT_THROW(ts.last_index(Date(2009, 12, 31)), std::out_of_range);
    ASSERT_EQ(0u, ts.last_index(Date(2010, 1, 2)));
    ASSERT_EQ(1u, ts.last_index(Date(2010, 1, 5)));
    ASSERT_EQ(0u, ts.last_index(d1));
    ASSERT_EQ(1u, ts.last_index(d2));
}

TEST(TimeSeries, FirstIndex) {
    const std::vector<double> values = { 0.1, 0.2 };
    const Date d1(2010, 1, 1);
    const Date d2(2010, 1, 4);
    std::vector<Date> times = { d1, d2 };
    time_series ts(times, values);
    ASSERT_THROW(ts.first_index(Date(2010, 1, 5)), std::out_of_range);
    ASSERT_EQ(0u, ts.first_index(Date(2009, 12, 31)));
    ASSERT_EQ(1u, ts.first_index(Date(2010, 1, 2)));
    ASSERT_EQ(0u, ts.first_index(d1));
    ASSERT_EQ(1u, ts.first_index(d2));
}

TEST(TimeSeries, IsNotATime) {
    ASSERT_FALSE(is_not_a_time(0.2));
    ASSERT_TRUE(is_not_a_time(std::numeric_limits<double>::quiet_NaN()));
    ASSERT_FALSE(is_not_a_time(Date(2011, 6, 23)));
    ASSERT_TRUE(is_not_a_time(Date()));
    ASSERT_FALSE(is_not_a_time(100));
}

TEST(TimeSeries, MoveConstructor) {
	const std::vector<double> values = { 0.1, 0.2 };
	const Date d1(2010, 1, 1);
	const Date d2(2010, 1, 2);
	const Date d; // not-a-time
	std::vector<Date> times = { d1, d2 };
	time_series ts1(times, values);
	time_series ts2(std::move(ts1));
	ASSERT_EQ(0u, ts1.size());
	ASSERT_EQ(2u, ts2.size());
	ASSERT_TRUE(ts1.empty());
    for (unsigned int i = 0; i < 2; ++i) {
        ASSERT_EQ(std::make_pair(times[i], values[i]), ts2[i]) << i;
    }
}

TEST(TimeSeries, MoveAssignment) {
	const std::vector<double> values = { 0.1, 0.2 };
	const Date d1(2010, 1, 1);
	const Date d2(2010, 1, 2);
	const Date d; // not-a-time
	std::vector<Date> times = { d1, d2 };
	time_series ts1(times, values);
	time_series ts2;
    ts2 = std::move(ts1);
	ASSERT_EQ(0u, ts1.size());
	ASSERT_EQ(2u, ts2.size());
	ASSERT_TRUE(ts1.empty());
    for (unsigned int i = 0; i < 2; ++i) {
        ASSERT_EQ(std::make_pair(times[i], values[i]), ts2[i]) << i;
    }
}

TEST(TimeSeries, ConstIterator) {
    const std::vector<double> values = { 0.1, 0.2 };
	const Date d1(2010, 1, 1);
	const Date d2(2010, 1, 2);
	std::vector<Date> times = { d1, d2 };
	time_series ts1(times, values);
    auto it = ts1.begin();
    ASSERT_NE(it, ts1.end());
    ASSERT_EQ(std::make_pair(d1, 0.1), *it);
    ++it;
    ASSERT_NE(it, ts1.end());
    ASSERT_EQ(std::make_pair(d2, 0.2), *it);
    --it;
    ASSERT_EQ(std::make_pair(d1, 0.1), *it);
    ++it; ++it;
    ASSERT_EQ(it, ts1.end());
    const auto comparison = [](Date d, std::pair<Date, double> p) { return d < p.first; };
    it = std::upper_bound(ts1.begin(), ts1.end(), d1, comparison);
    ASSERT_EQ(d2, (*it).first);
    it = std::upper_bound(ts1.begin(), ts1.end(), d2, comparison);
    ASSERT_EQ(it, ts1.end());
}

TEST(TimeSeries, Printing) {
    const std::vector<double> values = { 0.1, 0.2 };
	const Date d1(2010, 1, 1);
	const Date d2(2010, 1, 2);
	std::vector<Date> times = { d1, d2 };
	time_series ts(times, values);
    std::stringstream ss;
    ss << ts;
    ASSERT_EQ("[2010-01-01,0.1|2010-01-02,0.2]", ss.str());

	const std::vector<int8_t> values8 = { 0, -1 };
	TimeSeries<Date, int8_t> ts8(times, values8);
	ss = std::stringstream();
	ss << ts8;
	ASSERT_EQ("[2010-01-01,0|2010-01-02,-1]", ss.str());

	const std::vector<uint16_t> values16 = { 0, 1000 };
	TimeSeries<Date, uint16_t> ts16(times, values16);
	ss = std::stringstream();
	ss << ts16;
	ASSERT_EQ("[2010-01-01,0|2010-01-02,1000]", ss.str());
}

TEST(TimeSeries, Reading) {
    const std::vector<double> values = { 0.1, 0.2 };
	const Date d1(2010, 1, 1);
	const Date d2(2010, 1, 2);
	std::vector<Date> times = { d1, d2 };
	time_series ts1(times, values);
    time_series ts2 = time_series::from_string("[2010-Jan-01,0.1|2010-Jan-02,0.2]");
    ASSERT_EQ(ts1, ts2);
    ASSERT_THROW(ts2 = time_series::from_string("["), std::runtime_error);
    ASSERT_EQ(ts1, ts2);
    ASSERT_THROW(ts2 = time_series::from_string("]"), std::runtime_error);
    ASSERT_EQ(ts1, ts2);
    ASSERT_THROW(ts2 = time_series::from_string("[2010-Jan-02"), std::runtime_error);
    ASSERT_EQ(ts1, ts2);
    ASSERT_THROW(ts2 = time_series::from_string("[2010-Jan-02,"), std::runtime_error);
    ASSERT_EQ(ts1, ts2);
    ASSERT_THROW(ts2 = time_series::from_string("[2010-Jan-02,]"), std::runtime_error);
    ASSERT_EQ(ts1, ts2);
    ASSERT_THROW(ts2 = time_series::from_string("[2010-Jan-02,a]"), std::runtime_error);
    ASSERT_EQ(ts1, ts2);
    ASSERT_THROW(ts2 = time_series::from_string("[a,b]"), std::runtime_error);
    ASSERT_EQ(ts1, ts2);
    ts2 = time_series::from_string("[]");
    ASSERT_TRUE(ts2.empty());
    ts2 = time_series::from_string("[2010-01-01,0.1]");
    ASSERT_EQ(1u, ts2.size());
    ASSERT_EQ(ts1[0], ts2[0]);
}

TEST(TimeSeries, PaddedValue) {
	const std::vector<double> values = { 0.1, 0.2 };
	const Date d1(2010, 1, 1);
	const Date d2(2010, 2, 1);
	std::vector<Date> times = { d1, d2 };
	time_series ts1(times, values);
	ASSERT_EQ(0.1, ts1.padded_value(Date(2009, 1, 1)));
	ASSERT_EQ(0.1, ts1.padded_value(d1));
	ASSERT_EQ(0.1, ts1.padded_value(Date(2010, 1, 15)));
	ASSERT_EQ(0.2, ts1.padded_value(d2));
	ASSERT_EQ(0.2, ts1.padded_value(Date(2011, 1, 15)));
}

TEST(TimeSeries, ValueIterator) {
	const std::vector<double> values = { 0.1, 0.2 };
	const Date d1(2010, 1, 1);
	const Date d2(2010, 2, 1);
	std::vector<Date> times = { d1, d2 };
	time_series ts1(times, values);
	std::vector<double> v2(2);
	std::copy(ts1.values_begin(), ts1.values_end(), v2.begin());
	ASSERT_EQ(values, v2);
	for (time_series::ValueIterator it = ts1.values_begin(); it != ts1.values_end(); ++it) {
		(*it) *= 0.5;
	}
	ASSERT_NEAR(0.1, ts1[1].second, 1E-15);
}

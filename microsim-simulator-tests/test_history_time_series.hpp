/*
* (C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_MS_TEST_HISTORY_TIME_SERIES_H
#define __AVERISERA_MS_TEST_HISTORY_TIME_SERIES_H
#include <gtest/gtest.h>
#include "microsim-simulator/history/history_time_series.hpp"
#include <algorithm>
#include <cmath>

using namespace averisera;
using namespace averisera::microsim;

template <class T>
class TestHistoryTimeSeries : public testing::Test
{
};

TYPED_TEST_CASE_P(TestHistoryTimeSeries);

static const Date D0 = Date(2015, 3, 20);
static const Date D1 = Date(2015, 4, 1);
static const Date D2 = Date(2015, 4, 4);

TYPED_TEST_P(TestHistoryTimeSeries, Test) {
	HistoryTimeSeries<TypeParam> hist("Name");
	ASSERT_TRUE(hist.empty());
	ASSERT_EQ("Name", hist.name());
    ASSERT_EQ(0u, hist.size());
    ASSERT_THROW(hist.date(0), std::out_of_range);
    ASSERT_THROW(hist.as_double(0), std::out_of_range);
    ASSERT_THROW(hist.as_int(0), std::out_of_range);
	ASSERT_THROW(hist.last_date(), std::logic_error);
	ASSERT_THROW(hist.last_as_double(), std::logic_error);
	ASSERT_THROW(hist.last_as_int(), std::logic_error);
    ASSERT_THROW(hist.last_as_double(D1), std::out_of_range);
	ASSERT_THROW(hist.last_as_int(D1), std::out_of_range);
	ASSERT_EQ(FP_NAN, std::fpclassify(hist.as_double(D1)));
	ASSERT_THROW(hist.as_int(D1), std::runtime_error);
    
	const double original = 1.4;
	const TypeParam stored = static_cast<TypeParam>(original);
	const double as_double = static_cast<double>(stored);
	const History::int_t as_int = static_cast<History::int_t>(stored);    
	hist.append(D1, original);
    ASSERT_EQ(1u, hist.size());
    ASSERT_EQ(D1, hist.date(0));
    ASSERT_EQ(as_double, hist.as_double(0));
    ASSERT_EQ(as_int, hist.as_int(0));
    ASSERT_THROW(hist.date(1), std::out_of_range);
    ASSERT_THROW(hist.as_double(1), std::out_of_range);
    ASSERT_THROW(hist.as_int(1), std::out_of_range);
	ASSERT_FALSE(hist.empty());
	ASSERT_EQ(D1, hist.last_date());
	ASSERT_EQ(as_double, hist.last_as_double());
	ASSERT_EQ(as_int, hist.last_as_int());
    ASSERT_THROW(hist.last_as_double(D0), std::out_of_range);
	ASSERT_THROW(hist.last_as_int(D0), std::out_of_range);
	ASSERT_EQ(as_double, hist.last_as_double(D1));
	ASSERT_EQ(as_int, hist.last_as_int(D1));
	ASSERT_EQ(as_double, hist.last_as_double(D2));
	ASSERT_EQ(as_int, hist.last_as_int(D2));
	ASSERT_EQ(FP_NAN, std::fpclassify(hist.as_double(D0)));
	ASSERT_THROW(hist.as_int(D0), std::runtime_error);
	ASSERT_EQ(as_double, hist.as_double(D1));
	ASSERT_EQ(as_int, hist.as_int(D1));
	ASSERT_EQ(FP_NAN, std::fpclassify(hist.as_double(D2)));
	ASSERT_THROW(hist.as_int(D2), std::runtime_error);
	ASSERT_THROW(hist.append(D0, original), std::domain_error);
	ASSERT_THROW(hist.append(D1, original), std::domain_error);
    ASSERT_EQ(0u, hist.last_index(D1));
    ASSERT_EQ(0u, hist.first_index(D1));
    ASSERT_EQ(0u, hist.last_index(D2));
    ASSERT_THROW(hist.first_index(D2), std::out_of_range);
    ASSERT_THROW(hist.last_index(D0), std::out_of_range);
    ASSERT_EQ(0u, hist.first_index(D0));

	const HistoryData hd = hist.to_data();
	ASSERT_EQ(1, hd.size());
	ASSERT_EQ(D1, hd.dates()[0]);
	ASSERT_EQ(stored, hd.values().as<TypeParam>()[0]);
	ASSERT_EQ(print_type_name<TypeParam>(), hd.factory_type());
}


REGISTER_TYPED_TEST_CASE_P(TestHistoryTimeSeries, Test);

#endif // __AVERISERA_MS_TEST_HISTORY_TIME_SERIES_H

// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/history/history_time_series.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(HistoryTimeSeriesChar, Test) {
    HistoryTimeSeries<int8_t> h("name");
	ASSERT_EQ("name", h.name());
    ASSERT_TRUE(h.empty());
	const Date date(2000, 1, 1);
    h.append(date, static_cast<History::int_t>(10));
    ASSERT_FALSE(h.empty());
    ASSERT_THROW(h.append(Date(2001, 5, 13), (long int)1000), std::out_of_range);
	const HistoryData hd(h.to_data());
	ASSERT_EQ(1u, hd.size());
	ASSERT_EQ(date, hd.dates()[0]);
	ASSERT_EQ(10, hd.values().as<int8_t>()[0]);
}

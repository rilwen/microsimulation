#include <gtest/gtest.h>
#include "microsim-simulator/history/immutable_history_truncated.hpp"
#include "microsim-simulator/history/history_time_series.hpp"
#include <algorithm>
#include <cmath>

using namespace averisera;
using namespace averisera::microsim;

static const Date Dm1 = Date(2015, 3, 10);
static const Date D0 = Date(2015, 3, 20);
static const Date D1 = Date(2015, 4, 1);
static const Date D2 = Date(2015, 4, 4);


TEST(ImmutableHistoryTruncated, FromEmpty) {
    HistoryTimeSeries<double> orig("name");
    ImmutableHistoryTruncated trunc(orig, D1);
    ASSERT_TRUE(trunc.empty());
	ASSERT_EQ("name", trunc.name());
    ASSERT_EQ(trunc.size(), 0u);
    ASSERT_THROW(trunc.date(0), std::out_of_range);
    ASSERT_THROW(trunc.as_double(0), std::out_of_range);
    ASSERT_THROW(trunc.as_int(0), std::out_of_range);
}

TEST(ImmutableHistoryTruncated, TruncatedAtEvent) {
    HistoryTimeSeries<double> orig("name");
    orig.append(D0, 0.1);
    orig.append(D1, 0.11);
    orig.append(D2, 0.12);
    ImmutableHistoryTruncated trunc(orig, D1);
	ASSERT_EQ("name", trunc.name());
    ASSERT_FALSE(trunc.empty());
    ASSERT_EQ(0.1, trunc.as_double(D0));
    ASSERT_EQ(0.11, trunc.as_double(D1));
    ASSERT_EQ(0, trunc.as_int(D1));
    ASSERT_TRUE(std::isnan(trunc.as_double(D2)));
    ASSERT_THROW(trunc.as_int(D2), std::runtime_error);
    ASSERT_EQ(2u, trunc.size());
    ASSERT_EQ(D1, trunc.last_date());
    ASSERT_EQ(D0, trunc.date(0));
    ASSERT_EQ(0.1, trunc.as_double(0));
    ASSERT_EQ(0, trunc.as_int(0));
    ASSERT_EQ(D1, trunc.date(1));
    ASSERT_EQ(0.11, trunc.as_double(1));
    ASSERT_EQ(0, trunc.as_int(1));
    ASSERT_THROW(trunc.date(2), std::out_of_range);
    ASSERT_THROW(trunc.as_double(2), std::out_of_range);
    ASSERT_THROW(trunc.as_int(2), std::out_of_range);
    ASSERT_EQ(0u, trunc.first_index(D0));
    ASSERT_EQ(0u, trunc.last_index(D0));
    ASSERT_EQ(1u, trunc.last_index(D1));
    ASSERT_EQ(1u, trunc.first_index(D1));
    ASSERT_EQ(1u, trunc.last_index(D2));
    ASSERT_THROW(trunc.first_index(D2), std::out_of_range);
    ASSERT_THROW(trunc.last_index(Dm1), std::out_of_range);

	const HistoryData hd(trunc.to_data());
	const HistoryData hdorig(orig.to_data());
	ASSERT_EQ(2u, hd.size());
	ASSERT_EQ(D1, hd.dates()[1]);
	ASSERT_EQ(hdorig.factory_type(), hd.factory_type());
	ASSERT_EQ(hdorig.values().type(), hd.values().type());
}

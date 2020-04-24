/*
  (C) Averisera Ltd 2015
*/
#include <gtest/gtest.h>
#include "microsim-simulator/history/history_sparse.hpp"
#include "microsim-simulator/history/history_time_series.hpp"
#include <cmath>

using namespace averisera;
using namespace averisera::microsim;

static const Date D0 = Date(2015, 3, 20);
static const Date D1 = Date(2015, 4, 1);
static const Date D2 = Date(2015, 4, 4);
static const Date D3 = Date(2015, 4, 5);
static const Date D4 = Date(2015, 4, 6);
static const Date D5 = Date(2015, 4, 7);

TEST(HistorySparse, Test) {
	HistorySparse hist(std::unique_ptr<History>(new HistoryTimeSeries<double>("NAME")));
	ASSERT_EQ("NAME", hist.name());
	ASSERT_TRUE(hist.empty());
    ASSERT_EQ(0u, hist.size());
    ASSERT_THROW(hist.date(0), std::out_of_range);
	ASSERT_THROW(hist.last_date(), std::logic_error);
	ASSERT_THROW(hist.last_as_double(), std::logic_error);
	ASSERT_THROW(hist.last_as_int(), std::logic_error);
    ASSERT_THROW(hist.last_as_double(D1), std::out_of_range);
	ASSERT_THROW(hist.last_as_int(D1), std::out_of_range);
	ASSERT_EQ(FP_NAN, std::fpclassify(hist.as_double(D1)));
	ASSERT_THROW(hist.as_int(D1), std::runtime_error);
	const double stored = 1.4;
	const double as_double = static_cast<double>(stored);
	const double as_int = static_cast<int>(stored);
	hist.append(D1, stored);
    ASSERT_EQ(D1, hist.date(0));
    ASSERT_THROW(hist.date(1), std::out_of_range);
    ASSERT_EQ(1u, hist.size());
	ASSERT_FALSE(hist.empty());
    ASSERT_EQ(stored, hist.as_double(0));
    ASSERT_EQ(as_int, hist.as_int(0));
    ASSERT_THROW(hist.as_double(1), std::out_of_range);
    ASSERT_THROW(hist.as_int(1), std::out_of_range);
	ASSERT_EQ(D1, hist.last_date());
	ASSERT_EQ(as_double, hist.last_as_double());
	ASSERT_EQ(as_int, hist.last_as_int());
    ASSERT_THROW(hist.last_as_double(D0), std::out_of_range);
	ASSERT_THROW(hist.last_as_int(D0), std::out_of_range);        
	ASSERT_EQ(as_double, hist.last_as_double(D1));
	ASSERT_EQ(as_int, hist.last_as_int(D1));
	ASSERT_EQ(as_double, hist.last_as_double(D2));
	ASSERT_EQ(as_int, hist.last_as_int(D2));
    ASSERT_THROW(hist.last_as_double(D0), std::out_of_range);    
	ASSERT_THROW(hist.as_int(D0), std::runtime_error);
	ASSERT_EQ(as_double, hist.as_double(D1));
	ASSERT_EQ(as_int, hist.as_int(D1));
    ASSERT_EQ(FP_NAN, std::fpclassify(hist.as_double(D2)));
	ASSERT_THROW(hist.as_int(D2), std::runtime_error);
	ASSERT_THROW(hist.append(D0, stored), std::domain_error);
	ASSERT_THROW(hist.append(D1, stored), std::domain_error);

    hist.append(D2, stored);
    ASSERT_EQ(D2, hist.last_date());
    ASSERT_EQ(1u, hist.size());
    ASSERT_EQ(stored, hist.as_double(0));
    ASSERT_THROW(hist.as_int(1), std::out_of_range);
    ASSERT_EQ(as_double, hist.as_double(D2));
    ASSERT_EQ(as_double, hist.last_as_double(D2));
    ASSERT_EQ(as_int, hist.as_int(D2));
    ASSERT_EQ(as_int, hist.last_as_int(D2));

    const double stored2 = 2.6;
	const double as_double2 = static_cast<double>(stored2);
	const double as_int2 = static_cast<int>(stored2);
    ASSERT_EQ(FP_NAN, std::fpclassify(hist.as_double(D3)));
    ASSERT_THROW(hist.as_int(D3), std::runtime_error);

    hist.append(D4, stored2);
    ASSERT_EQ(2u, hist.size());
    ASSERT_EQ(D4, hist.date(1));
    ASSERT_EQ(stored2, hist.as_double(1));
    ASSERT_EQ(as_int2, hist.as_int(1));
    ASSERT_THROW(hist.as_int(2), std::out_of_range);
    ASSERT_EQ(D4, hist.last_date());
    ASSERT_EQ(as_double, hist.as_double(D3));
    ASSERT_EQ(as_double, hist.last_as_double(D3));
    ASSERT_EQ(as_int, hist.as_int(D3));
    ASSERT_EQ(as_int, hist.last_as_int(D3));
    ASSERT_EQ(as_double2, hist.as_double(D4));
    ASSERT_EQ(as_double2, hist.last_as_double(D4));
    ASSERT_EQ(as_int2, hist.as_int(D4));
    ASSERT_EQ(as_int2, hist.last_as_int(D4));
    ASSERT_EQ(1u, hist.last_index(D4));
    ASSERT_EQ(1u, hist.first_index(D4));
    ASSERT_EQ(0u, hist.last_index(D2));
    ASSERT_EQ(1u, hist.first_index(D2));
    ASSERT_THROW(hist.first_index(D5), std::out_of_range);
    ASSERT_THROW(hist.last_index(D0), std::out_of_range);

	const HistoryData hd(hist.to_data());
	ASSERT_EQ(2, hd.size());
	ASSERT_EQ(D1, hd.dates()[0]);
	ASSERT_EQ(D4, hd.dates()[1]);
	ASSERT_EQ(stored, hd.values().as<double>()[0]);
	ASSERT_EQ(stored2, hd.values().as<double>()[1]);
	ASSERT_EQ("sparse double", hd.factory_type());
	ASSERT_EQ(ObjectVector::Type::DOUBLE, hd.values().type());
}

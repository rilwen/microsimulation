/*
* (C) Averisera Ltd 2015
*/

#include "test_history_time_series.hpp"

typedef ::testing::Types<int32_t, double> MyTypes;
INSTANTIATE_TYPED_TEST_CASE_P(My, TestHistoryTimeSeries, MyTypes);

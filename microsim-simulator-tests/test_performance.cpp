/*
(C) Averisera Ltd 2017
*/
#include <gtest/gtest.h>
#include "microsim-simulator/performance.hpp"
#include <chrono>
#include <thread>

using namespace averisera;
using namespace averisera::microsim;

TEST(Performance, AppendMetrics) {
	Performance p;
	p.append_metrics(0.2, 2);
	ASSERT_EQ(2u, p.total_nbr_processed());
	ASSERT_EQ(2.0, p.nbr_processed_stats().mean());
	ASSERT_EQ(0.2, p.total_time_stats().mean());
	ASSERT_EQ(0.2, p.total_time());
	ASSERT_NEAR(0.1, p.time_per_element_stats().mean(), 1e-12);
	p.append_metrics(0.3, 1);
	ASSERT_EQ(3u, p.total_nbr_processed());
	ASSERT_EQ(0.5, p.total_time());
	ASSERT_THROW(p.append_metrics(-0.1, 2), std::domain_error);
	ASSERT_THROW(p.append_metrics(0.1, 0), std::domain_error);
}

TEST(Performance, MeasureMetrics) {
	Performance p;
	const int n = 10000;
	p.measure_metrics([n]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}, n);
	ASSERT_EQ(n, p.total_nbr_processed());
	ASSERT_EQ(n, p.nbr_processed_stats().mean());
	ASSERT_TRUE(std::isfinite(p.time_per_element_stats().mean()));
	ASSERT_GT(p.total_time_stats().mean(), 0.0);
	ASSERT_EQ(p.total_time_stats().mean(), p.total_time());
	ASSERT_NEAR(p.total_time_stats().mean() / n, p.time_per_element_stats().mean(), 1e-10);
}

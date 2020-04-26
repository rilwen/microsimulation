// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-calibrator/rate_calibrator.hpp"
#include "core/data_exception.hpp"
#include "core/discrete_distribution.hpp"
#include "core/rng_impl.hpp"
#include "core/running_statistics.hpp"
#include <cassert>

using namespace averisera;
using namespace averisera::microsim;

TEST(RateCalibrator, realign_matrix_to_years) {
    Eigen::MatrixXd rates(3, 2);
    rates << 0.001, 0.002,
        0.003, 0.004,
        0.005, 0.006;
    const std::vector<int> years({1980, 1985, 1990});
    const std::vector<int> new_years({1975, 1980, 1982, 1985, 1986, 1987, 1990, 1992, 1994});
    const Eigen::MatrixXd new_rates = RateCalibrator::realign_matrix_to_years(rates, years, new_years);
    ASSERT_EQ(new_years.size(), static_cast<size_t>(new_rates.rows()));
    ASSERT_EQ(rates.cols(), new_rates.cols());
    Eigen::MatrixXd expected_new_rates(9, 2);
    expected_new_rates << 0.001, 0.002,
        0.001, 0.002,
        0.001, 0.002,
        0.003, 0.004,
        0.003, 0.004,
        0.003, 0.004,
        0.005, 0.006,
        0.005, 0.006,
        0.005, 0.006;
    ASSERT_NEAR(0.0, (expected_new_rates - new_rates).norm(), 1E-12);
}

TEST(RateCalibrator, merge_years) {
    const std::vector<int> years1({1990, 1991, 1994, 1995});
    const std::vector<int> years2({1989, 1990, 1993, 1994});
    const std::vector<int> years(RateCalibrator::merge_years(years1, years2));
    ASSERT_EQ(std::vector<int>({1989, 1990, 1991, 1993, 1994, 1995}), years);
}

TEST(RateCalibrator, average_neighbouring_years) {
	Eigen::MatrixXd values(3, 2);
	values << 890, 990,
		910, 1010,
		930, 1030;
	const std::vector<int> years({ 2000, 2001, 2002 });
	std::vector<int> new_years;
	Eigen::MatrixXd new_values;
	RateCalibrator::average_neighbouring_years(values, years, true, new_values, new_years);
	Eigen::MatrixXd expected(2, 2);
	expected << 900, 1000,
		920, 1020;
	ASSERT_NEAR(0.0, (expected - new_values).norm(), 1E-13);
	ASSERT_EQ(std::vector<int>({ 2001, 2002 }), new_years);
	RateCalibrator::average_neighbouring_years(values, years, false, new_values, new_years);
	ASSERT_NEAR(0.0, (expected - new_values).norm(), 1E-13);
	ASSERT_EQ(std::vector<int>({ 2000, 2001 }), new_years);
}

TEST(RateCalibrator, simulate_avg_number_jumps) {
	const unsigned int iters = 100000;
	const double T = 5; 
	const double h0 = 0.2;
	RNGImpl rng(42);
	const RunningStatistics<double> res = RateCalibrator::simulate_avg_number_jumps(h0, T, 0.0, iters, rng);
	ASSERT_NEAR(T * h0, res.mean(), 2 * res.standard_deviation() / sqrt(static_cast<double>(res.nbr_samples())));
}

TEST(RateCalibrator, simulate_avg_increment) {
	const unsigned int iters = 100000;
	const double T = 5;
	const double h0 = 0.2;
	RNGImpl rng(42);
	const DiscreteDistribution jds(1, std::vector<double>({ 0.8, 0.15, 0.05 }));
	RunningStatistics<double> res = RateCalibrator::simulate_avg_increment(h0, T, 0.0, iters, jds, rng);
	ASSERT_NEAR(jds.mean() * T * h0, res.mean(), 2 * res.standard_deviation() / sqrt(static_cast<double>(res.nbr_samples())));
	const DiscreteDistribution jds1(1, std::vector<double>({ 1.0 }));
	ASSERT_EQ(1, jds1.mean());
	res = RateCalibrator::simulate_avg_increment(h0, T, 0.0, iters, jds1, rng);
	ASSERT_NEAR(T * h0, res.mean(), 2.1 * res.standard_deviation() / sqrt(static_cast<double>(res.nbr_samples())));
}

TEST(RateCalibrator, hazard_rate_from_average_increment) {
	double dt = 0.75;
	const double T = 5;
	const unsigned int iters = 50000;
	//const double h0 = 0.2;
    const DiscreteDistribution jds1(1, std::vector<double>({1.0}));
    ASSERT_EQ(1, jds1.mean());
    const DiscreteDistribution jds(1, std::vector<double>({0.8, 0.15, 0.05}));
	RNGImpl rng(42);
	double N = 1.5;
	double h = RateCalibrator::hazard_rate_from_average_increment(T, N, dt, jds);
	RunningStatistics<double> res = RateCalibrator::simulate_avg_increment(h, T, dt, iters, jds, rng);
	EXPECT_NEAR(N, res.mean(), 2 * res.standard_deviation() / sqrt(static_cast<double>(res.nbr_samples())));
    h = RateCalibrator::hazard_rate_from_average_increment(T, N, dt, jds1);
	res = RateCalibrator::simulate_avg_increment(h, T, dt, iters, jds1, rng);
	ASSERT_NEAR(N, res.mean(), 2 * res.standard_deviation() / sqrt(static_cast<double>(res.nbr_samples()))) << "jds1";
	N = 0.01;
	h = RateCalibrator::hazard_rate_from_average_increment(T, N, dt, jds);
	res = RateCalibrator::simulate_avg_increment(h, T, dt, iters, jds, rng);
	EXPECT_NEAR(N, res.mean(), 2 * res.standard_deviation() / sqrt(static_cast<double>(res.nbr_samples())));
	dt = 0.1;
	N = 10;
	h = RateCalibrator::hazard_rate_from_average_increment(T, N, dt, jds);
	res = RateCalibrator::simulate_avg_increment(h, T, dt, iters, jds, rng);
	EXPECT_NEAR(N, res.mean(), 2 * res.standard_deviation() / sqrt(static_cast<double>(res.nbr_samples())));
	dt = 4;
	N = 0.5;
	h = RateCalibrator::hazard_rate_from_average_increment(T, N, dt, jds);
	res = RateCalibrator::simulate_avg_increment(h, T, dt, iters, jds, rng);
	EXPECT_NEAR(N, res.mean(), 2 * res.standard_deviation() / sqrt(static_cast<double>(res.nbr_samples())));
}

TEST(RateCalibrator, compare_variances) {
	const double T = 5;
	const unsigned int iters = 100000;
	const double dt = 1;
	const double N = 1.5;
	const double h0 = RateCalibrator::hazard_rate_from_average_occurrences(T, N, 0.0, iters);
	const double h = RateCalibrator::hazard_rate_from_average_occurrences(T, N, dt, iters);
	const long seed = 42;
	RNGImpl rng(seed);
	const RunningStatistics<double> res0 = RateCalibrator::simulate_avg_number_jumps(h0, T, 0.0, iters, rng);
	rng = RNGImpl(seed);
	const RunningStatistics<double> res = RateCalibrator::simulate_avg_number_jumps(h, T, dt, iters, rng);
	const double f = sqrt(static_cast<double>(iters) - 1.0);
	const double n_st_dev = 3.0; // for tests
	ASSERT_NEAR(N, res.mean(), n_st_dev * res.standard_deviation() / f);
	ASSERT_NEAR(N, res0.mean(), n_st_dev * res0.standard_deviation() / f);
	std::cout << "dt=" << dt << ": " << res.mean() << ", " << res.variance() << "\n";
	std::cout << "dt=" << 0 << ": " << res0.mean() << ", " << res0.variance() << "\n";
	const double g = sqrt((static_cast<double>(iters) - 1) / 2.0); // scaling factor for standard deviation of the estimator of variance, see https://web.eecs.umich.edu/~fessler/papers/files/tr/stderr.pdf
	ASSERT_GT(res0.variance(), res.variance() * (1 + n_st_dev / g));
}

TEST(RateCalibrator, hazard_rate_from_average_occurrences) {
	double dt = 0.75;
	double T = 5;
	const unsigned int iters = 50000;
	//const double h0 = 0.2;
	RNGImpl rng(42);
	double N = 1.5;
	double h = RateCalibrator::hazard_rate_from_average_occurrences(T, N, dt);
    ASSERT_GT(h, N / T);
	RunningStatistics<double> res = RateCalibrator::simulate_avg_number_jumps(h, T, dt, iters, rng);
	EXPECT_NEAR(N, res.mean(), 2 * res.standard_deviation() / sqrt(static_cast<double>(res.nbr_samples())));
	N = 0.008;
	h = RateCalibrator::hazard_rate_from_average_occurrences(T, N, dt);
    EXPECT_GE(h, N / T);
	res = RateCalibrator::simulate_avg_number_jumps(h, T, dt, iters, rng);
	EXPECT_NEAR(N, res.mean(), 2 * res.standard_deviation() / sqrt(static_cast<double>(res.nbr_samples())));
	dt = 0.1;
	N = 10;
	h = RateCalibrator::hazard_rate_from_average_occurrences(T, N, dt);
    ASSERT_GT(h, N / T);
	res = RateCalibrator::simulate_avg_number_jumps(h, T, dt, iters, rng);
	EXPECT_NEAR(N, res.mean(), 2 * res.standard_deviation() / sqrt(static_cast<double>(res.nbr_samples())));
	dt = 4;
	N = 0.5;
	h = RateCalibrator::hazard_rate_from_average_occurrences(T, N, dt);
    ASSERT_GT(h, N / T);
	res = RateCalibrator::simulate_avg_number_jumps(h, T, dt, iters, rng);
	EXPECT_NEAR(N, res.mean(), 2 * res.standard_deviation() / sqrt(static_cast<double>(res.nbr_samples())));
	N = 0.0012851306905774004;
	T = 1.0;
	dt = 0.75;
	h = RateCalibrator::hazard_rate_from_average_occurrences(T, N, dt);
	EXPECT_GE(h, N / T);
	res = RateCalibrator::simulate_avg_number_jumps(h, T, dt, iters, rng);
	EXPECT_NEAR(N, res.mean(), 2 * res.standard_deviation() / sqrt(static_cast<double>(res.nbr_samples())));
}


TEST(RateCalibrator, aggregate_age_groups) {
	Eigen::MatrixXd values(2, 4);
	values << 1, 2, 3, 4,
		5, 6, 7, 8;
	const std::vector<int> index({ 1,2 });
	typedef RateCalibrator::age_group_type age_group_type;	
	typedef DataFrame<age_group_type, int> df;
	const std::vector<age_group_type> old_groups({ age_group_type(0, 9), age_group_type(10, 19), age_group_type(25, 29), age_group_type(30, 34) });
	const std::vector<age_group_type> new_groups({ age_group_type(0, 19), age_group_type(20, 24), age_group_type(25, 34) });
	df values_df(values, old_groups, index);
	df new_values = RateCalibrator::aggregate_age_groups(values_df, new_groups);
	ASSERT_EQ(values.rows(), new_values.nbr_rows());
	ASSERT_EQ(new_groups.size(), new_values.nbr_cols());
	Eigen::MatrixXd expected(values.rows(), new_groups.size());
	expected.setZero();
	expected.col(0).noalias() += values.col(0);
	expected.col(0).noalias() += values.col(1);
	expected.col(2).noalias() += values.col(2);
	expected.col(2).noalias() += values.col(3);
	ASSERT_NEAR(0.0, (expected - new_values.values()).norm(), 1E-14);
	ASSERT_THROW(RateCalibrator::aggregate_age_groups(values_df, std::vector<age_group_type>({ age_group_type(1, 19), age_group_type(20, 24), age_group_type(25, 34) })), DataException);
	ASSERT_THROW(RateCalibrator::aggregate_age_groups(values_df, std::vector<age_group_type>({ age_group_type(0, 19), age_group_type(20, 24), age_group_type(25, 33) })), DataException);
	ASSERT_THROW(RateCalibrator::aggregate_age_groups(values_df, std::vector<age_group_type>({ age_group_type(0, 21), age_group_type(20, 24), age_group_type(25, 34) })), DataException);
	ASSERT_THROW(RateCalibrator::aggregate_age_groups(values_df, std::vector<age_group_type>({ age_group_type(0, 19), age_group_type(20, 27), age_group_type(28, 34) })), DataException);
	new_values = RateCalibrator::aggregate_age_groups(values_df, std::vector<age_group_type>());
	ASSERT_EQ(0, new_values.nbr_cols());
}

TEST(RateCalibrator, make_age_ranges) {
	auto ranges = RateCalibrator::make_age_ranges(2, 6);
	ASSERT_EQ(std::vector<RateCalibrator::age_group_type>({
		RateCalibrator::age_group_type(0, 2), RateCalibrator::age_group_type(2, 4), RateCalibrator::age_group_type(4, 6), RateCalibrator::age_group_type(6, RateCalibrator::MAX_AGE + 1)
	}), ranges);
	ranges = RateCalibrator::make_age_ranges(2, 6, 1);
	ASSERT_EQ(std::vector<RateCalibrator::age_group_type>({
		RateCalibrator::age_group_type(1, 3), RateCalibrator::age_group_type(3, 5), RateCalibrator::age_group_type(5, 7), RateCalibrator::age_group_type(7, RateCalibrator::MAX_AGE + 1)
	}), ranges);
}

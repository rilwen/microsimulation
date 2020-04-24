/*
(C) Averisera Ltd 2014
*/
#include <gtest/gtest.h>
#include "core/observed_discrete_data.hpp"
#include "core/bootstrap.hpp"
#include "testing/temporary_file.hpp"
#include <fstream>

using namespace averisera;


TEST(ObservedDiscreteData, DefaultConstructor) {
	averisera::ObservedDiscreteData data(0);
	ASSERT_EQ(0, data.probs.size());
	ASSERT_EQ(0, data.nbr_surveys.size());
	ASSERT_EQ(0u, data.times.size());
}

TEST(ObservedDiscreteData, MoveConstructor) {
	averisera::ObservedDiscreteData d1(2, 3);
	d1.probs.setConstant(0.5);
	averisera::ObservedDiscreteData d2(std::move(d1));
	ASSERT_EQ(d2.nbr_surveys.size(), 3);
	ASSERT_EQ(d2.probs(1, 2), 0.5);
	ASSERT_EQ(d2.times.size(), 3u);
}

TEST(ObservedDiscreteData, ConstructWithDimAndT) {
	averisera::ObservedDiscreteData data(2, 3);
	ASSERT_EQ(2, data.probs.rows());
	ASSERT_EQ(3, data.probs.cols());
	ASSERT_EQ(3, data.nbr_surveys.size());
	ASSERT_EQ(3u, data.times.size());
	for (size_t i = 0; i < 3; ++i) {
		ASSERT_EQ(1.0, data.nbr_surveys[i]) << i;
		ASSERT_EQ(i, data.times[i]) << i;
	}
}

TEST(ObservedDiscreteData, Pad) {
	averisera::ObservedDiscreteData data(2, 2);
	data.probs(0, 0) = 1.0;
	data.probs(1, 0) = 0.0;
	data.probs(0, 1) = 0.5;
	data.probs(1, 1) = 0.5;
	data.nbr_surveys[0] = 100;
	data.nbr_surveys[1] = 200;
	data.times[0] = 2000;
	data.times[1] = 2002;
	data.ltrajs.init_rectangular(1, 2);
	data.ltimes.init_rectangular(1, 2);
	data.ltrajs(0, 0) = 0;
	data.ltrajs(0, 1) = 0;
	data.ltimes(0, 0) = 1999;
	data.ltimes(0, 1) = 2000;
	std::vector<size_t> input_to_padded;
	auto padded_data = averisera::ObservedDiscreteData::pad(data, input_to_padded);
	ASSERT_EQ(4u, static_cast<unsigned int>(padded_data.nbr_surveys.size()));
	ASSERT_EQ(4u, padded_data.times.size());
	ASSERT_EQ(2, padded_data.probs.rows());
	ASSERT_EQ(4, padded_data.probs.cols());
	for (size_t t = 0; t < 4; ++t) {
		ASSERT_EQ(1999 + t, padded_data.times[t]) << t;
	}
	ASSERT_EQ(0, padded_data.nbr_surveys[0]);
	ASSERT_EQ(data.nbr_surveys[0], padded_data.nbr_surveys[1]);
	ASSERT_EQ(0, padded_data.nbr_surveys[2]);
	ASSERT_EQ(data.nbr_surveys[1], padded_data.nbr_surveys[3]);
	ASSERT_EQ(std::vector<size_t>({ 1, 3 }), input_to_padded);
	ASSERT_EQ(Eigen::VectorXd::Ones(2) * 0.5, padded_data.probs.col(0));
	ASSERT_EQ(data.probs.col(0), padded_data.probs.col(1));
	ASSERT_EQ(data.probs.col(0), padded_data.probs.col(2));
	ASSERT_EQ(data.probs.col(1), padded_data.probs.col(3));

	data.probs.resize(0, 0);
	data.nbr_surveys.resize(0);
	data.times.resize(0);
	padded_data = averisera::ObservedDiscreteData::pad(data, input_to_padded);
	ASSERT_TRUE(input_to_padded.empty());
	ASSERT_EQ(2u, padded_data.times.size());
	ASSERT_EQ(2u, static_cast<unsigned int>(padded_data.nbr_surveys.size()));
	ASSERT_EQ(1, padded_data.probs.rows());
	ASSERT_EQ(2, padded_data.probs.cols());
	for (size_t t = 0; t < 2; ++t) {
		ASSERT_EQ(1999 + t, padded_data.times[t]) << t;
		ASSERT_EQ(0, padded_data.nbr_surveys[t]) << t;
		ASSERT_EQ(1, padded_data.probs(0, t)) << t;
	}
}

TEST(ObservedDiscreteData, ChangeTimeUnit) {
	averisera::ObservedDiscreteData data(2, 2);
	averisera::ObservedDiscreteData::change_time_unit(data, 0.5);
	ASSERT_EQ(2u, data.times.size());
	ASSERT_EQ(0, data.times[0]);
	ASSERT_EQ(2, data.times[1]);
}

TEST(ObservedDiscreteData, SmallestTimeIncrementVector) {
	std::vector<double> times = { 0, 1, 1.5, 2.5 };
	ASSERT_EQ(0.5, averisera::ObservedDiscreteData::smallest_time_increment(times));
	times.resize(1);
	ASSERT_EQ(std::numeric_limits<double>::infinity(), averisera::ObservedDiscreteData::smallest_time_increment(times));
	times.resize(0);
	ASSERT_EQ(std::numeric_limits<double>::infinity(), averisera::ObservedDiscreteData::smallest_time_increment(times));
}

TEST(ObservedDiscreteData, DiscretizeTimes) {
	averisera::ObservedDiscreteData data(1, 3);
	data.times = { 0, 1, 1.5 };
	data.ltimes = averisera::Jagged2DArray<double>(1, 2);
	data.ltimes(0, 0) = 0.25;
	data.ltimes(0, 1) = 0.5;
	const double dt = averisera::ObservedDiscreteData::discretize_times(data);
	ASSERT_EQ(std::vector<double>({ 0, 4, 6 }), data.times);
	ASSERT_EQ(1, data.ltimes(0, 0));
	ASSERT_EQ(2, data.ltimes(0, 1));
	ASSERT_EQ(0.25, dt);
}

TEST(ObservedDiscreteData, FirstLastTime) {
	averisera::ObservedDiscreteData data(1, 3);
	data.times = { 0, 1, 1.5 };
	data.ltimes = averisera::Jagged2DArray<double>(2, 2);
	data.ltimes(0, 0) = 0;
	data.ltimes(0, 1) = 2;
	data.ltimes(1, 0) = -1;
	data.ltimes(1, 1) = 0.5;
	const double t0 = averisera::ObservedDiscreteData::first_time(data);
	const double t1 = averisera::ObservedDiscreteData::last_time(data);
	ASSERT_EQ(-1., t0);
	ASSERT_EQ(2.0, t1);
}

TEST(ObservedDiscreteData, Resample) {
	averisera::Bootstrap<> bootstrap;
	const size_t dim = 2;
	const size_t T = 2;
	averisera::ObservedDiscreteData data(dim, T);
	data.probs(0, 0) = 1.0;
	data.probs(1, 0) = 0.0;
	data.probs(0, 1) = 0.5;
	data.probs(1, 1) = 0.5;
	data.nbr_surveys[0] = 100;
	data.nbr_surveys[1] = 200;
	data.times[0] = 2000;
	data.times[1] = 2002;
	data.ltrajs.init_rectangular(2, 2);
	data.ltimes.init_rectangular(2, 2);
	data.ltrajs(0, 0) = 0;
	data.ltrajs(0, 1) = 0;
	data.ltimes(0, 0) = 1999;
	data.ltimes(0, 1) = 2000;
	data.ltrajs(1, 0) = 1;
	data.ltrajs(1, 1) = 1;
	data.ltimes(1, 0) = 2000;
	data.ltimes(1, 1) = 2001;
	averisera::ObservedDiscreteData resampled(averisera::ObservedDiscreteData::resample(bootstrap, data));
	ASSERT_EQ(dim, averisera::ObservedDiscreteData::dim(resampled));	
	ASSERT_EQ(T, static_cast<size_t>(resampled.probs.cols()));
	for (size_t t = 0; t < T; ++t) {
		ASSERT_NEAR(1.0, resampled.probs.col(t).sum(), 1E-14) << t;
	}
	ASSERT_EQ(2u, resampled.ltimes.size());
	ASSERT_EQ(2u, resampled.ltrajs.size());
	ASSERT_EQ(data.nbr_surveys, resampled.nbr_surveys);
	ASSERT_EQ(data.times, resampled.times);
	for (size_t i = 0; i < 2; ++i) {
		const bool eq0 = resampled.ltrajs[i] == data.ltrajs[0];
		const bool eq1 = resampled.ltrajs[i] == data.ltrajs[1];
		ASSERT_TRUE(eq0 || eq1) << i;
		const size_t orig_idx = eq0 ? 0 : 1;
		ASSERT_EQ(data.ltimes[orig_idx], resampled.ltimes[i]) << i;
	}
}

TEST(ObservedDiscreteData, ToCrossSectional) {
	const size_t dim = 2;
	const size_t T = 2;
	averisera::ObservedDiscreteData data(dim, T);
	data.probs(0, 0) = 1.0;
	data.probs(1, 0) = 0.0;
	data.probs(0, 1) = 0.5;
	data.probs(1, 1) = 0.5;
	data.nbr_surveys[0] = 2;
	data.nbr_surveys[1] = 4;
	data.times[0] = 2000;
	data.times[1] = 2002;
	data.ltrajs.init_rectangular(2, 2);
	data.ltimes.init_rectangular(2, 2);
	data.ltrajs(0, 0) = 0;
	data.ltrajs(0, 1) = 0;
	data.ltimes(0, 0) = 1999;
	data.ltimes(0, 1) = 2000;
	data.ltrajs(1, 0) = 1;
	data.ltrajs(1, 1) = 1;
	data.ltimes(1, 0) = 2000;
	data.ltimes(1, 1) = 2001;
	averisera::ObservedDiscreteData reduced(averisera::ObservedDiscreteData::to_cross_sectional(data, 1.0));
	ASSERT_EQ(dim, averisera::ObservedDiscreteData::dim(reduced));
	ASSERT_EQ(4u, reduced.times.size());
	for (size_t t = 0; t < 4; ++t) {
		ASSERT_EQ(1999 + t, reduced.times[t]) << t;
	}
	ASSERT_EQ(1, reduced.nbr_surveys[0]);
	ASSERT_EQ(4, reduced.nbr_surveys[1]);
	ASSERT_EQ(1, reduced.nbr_surveys[2]);
	ASSERT_EQ(4, reduced.nbr_surveys[3]);
	Eigen::VectorXd v(2);
	v << 1.0, 0.0; ASSERT_EQ(v, reduced.probs.col(0));
	v << 0.75, 0.25; ASSERT_EQ(v, reduced.probs.col(1));
	v << 0.0, 1.0; ASSERT_EQ(v, reduced.probs.col(2));
	v << 0.5, 0.5; ASSERT_EQ(v, reduced.probs.col(3));

	reduced = averisera::ObservedDiscreteData::to_cross_sectional(data, 0.5);
	ASSERT_EQ(1u, reduced.ltimes.size());
	ASSERT_EQ(1u, reduced.ltrajs.size());
	ASSERT_EQ(8, reduced.nbr_surveys.sum());
}

TEST(ObservedDiscreteData, CountSpecifiedStates) {
	const std::vector<int> expanded_trajectory = { -1, 0, 0, -1, 0, -1, -1, 0 };
	const unsigned int memory = 1;
	const std::vector<unsigned int> expected = { 0, 1, 2, 1, 1, 1, 0, 1 };
	for (unsigned int k = 0; k < 8; ++k) {
		ASSERT_EQ(expected[k], averisera::ObservedDiscreteData::count_specified_states(expanded_trajectory, k, memory)) << k;
	}
}

TEST(ObservedDiscreteData, StateIndexCompatibleWithData) {
	const std::vector<int> expanded_trajectory = { -1, 0, 1, -1, 0, -1, -1, 1 };
	const unsigned int memory = 1;
	std::vector<size_t> indices = { 1, 0 }; // first one is the most recent
	const std::vector<bool> expected = { true, false, true, false, false, true, true, true };
	for (unsigned int k = 0; k < 8; ++k) {
		ASSERT_EQ(expected[k], averisera::ObservedDiscreteData::state_index_compatible_with_data(expanded_trajectory, indices, memory, k)) << k;
	}
}

template <class T, class V, class M> void assert_equal(const std::vector<T>& expected, const V& actual, const M& msg) {
	ASSERT_EQ(expected.size(), actual.size()) << msg;
	for (size_t i = 0; i < expected.size(); ++i) {
		ASSERT_EQ(expected[i], actual[i]) << msg << " " << i;
	}
}


TEST(ObservedDiscreteData, LoadProbabilities) {
	struct TemporaryFileWithData : public averisera::testing::TemporaryFile {
		TemporaryFileWithData() {
			std::ofstream outf(filename);
			outf << "0.2\t0.8\t100\t1990\n";
			outf << "0.3\t0.7\t110\t1991\n";
			outf << "0.4\t0.6\t120\t1992\n";
		}
	};
	TemporaryFileWithData file;
	ObservedDiscreteData data;
	const auto retval = ObservedDiscreteData::load_probabilities(file.filename.c_str(), data, true);
	ASSERT_EQ(2u, retval.first);
	ASSERT_EQ(3u, retval.second);
	ASSERT_EQ(2u, ObservedDiscreteData::dim(data));
	ASSERT_FALSE(ObservedDiscreteData::has_trajectories(data));
	ASSERT_EQ(3u, data.times.size());
	ASSERT_EQ(3u, static_cast<unsigned int>(data.nbr_surveys.size()));
	ASSERT_EQ(3, data.probs.cols());
	for (size_t t = 0; t < 3; ++t) {
		ASSERT_NEAR(0.2 + 0.1 * static_cast<double>(t), data.probs(0, t), 1E-15) << t;
		ASSERT_NEAR(0.8 - 0.1 * static_cast<double>(t), data.probs(1, t), 1E-15) << t;
		ASSERT_EQ(100 + 10 * static_cast<double>(t), data.nbr_surveys[t]) << t;
		ASSERT_EQ(1990 + static_cast<double>(t), data.times[t]) << t;
	}
}

TEST(ObservedDiscreteData, LoadTrajectories) {
	struct TemporaryFileWithData : public averisera::testing::TemporaryFile {
		TemporaryFileWithData() {
			std::ofstream outf(filename);
			outf << "1990\t1991\t1993\t1995\n";
			outf << "0\t1\t1\t0\n";
			outf << "0\t-1\t0\t-1\n";
			outf << "0\t1\n";
		}
	};
	TemporaryFileWithData file;
	ObservedDiscreteData data;
	const size_t nload = ObservedDiscreteData::load_trajectories(file.filename.c_str(), data);
	ASSERT_EQ(3u, nload);
	ASSERT_EQ(2u, ObservedDiscreteData::dim(data));
	ASSERT_TRUE(ObservedDiscreteData::has_trajectories(data));
	ASSERT_EQ(0u, static_cast<unsigned int>(data.probs.size()));
	ASSERT_EQ(0u, data.times.size());
	ASSERT_EQ(0u, static_cast<unsigned int>(data.nbr_surveys.size()));
	ASSERT_EQ(3u, data.ltrajs.size());
	ASSERT_EQ(3u, data.ltimes.size());
	assert_equal(std::vector<unsigned int>({ 0, 1, 1, 0 }), data.ltrajs[0], 0);
	assert_equal(std::vector<unsigned int>({ 0, 0 }), data.ltrajs[1], 1);
	assert_equal(std::vector<unsigned int>({ 0, 1 }), data.ltrajs[2], 2);
	assert_equal(std::vector<double>({ 1990, 1991, 1993, 1995 }), data.ltimes[0], 0);
	assert_equal(std::vector<double>({ 1990, 1993 }), data.ltimes[1], 1);
	assert_equal(std::vector<double>({ 1990, 1991 }), data.ltimes[2], 2);
}

/// Used to debug an addin problem - 2018/12/23.
//TEST(ObservedDiscreteData, LoadTrajectoriesMany) {
//	ObservedDiscreteData data;
//	const size_t nload = ObservedDiscreteData::load_trajectories("tests/resources/trajectories.tsv", data);
//	ASSERT_EQ(100u, nload);
//	ASSERT_EQ(3u, ObservedDiscreteData::dim(data));
//	ASSERT_TRUE(ObservedDiscreteData::has_trajectories(data));
//	ASSERT_EQ(0u, static_cast<unsigned int>(data.probs.size()));
//	ASSERT_EQ(0u, data.times.size());
//	ASSERT_EQ(0u, static_cast<unsigned int>(data.nbr_surveys.size()));
//	ASSERT_EQ(100u, data.ltrajs.size());
//	ASSERT_EQ(100u, data.ltimes.size());
//	assert_equal(std::vector<unsigned int>({ 1, 2, 1 }), data.ltrajs[0], 0);
//	assert_equal(std::vector<unsigned int>({ 2, 1, 1, 2 }), data.ltrajs[1], 1);
//	assert_equal(std::vector<double>({ 2000, 2003, 2004}), data.ltimes[0], 0);
//	assert_equal(std::vector<double>({ 2000, 2001, 2003, 2004}), data.ltimes[1], 1);
//}

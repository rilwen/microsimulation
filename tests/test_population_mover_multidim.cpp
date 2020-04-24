//#include <gtest/gtest.h>
//#include "core/beta_distribution.hpp"
//#include "core/distribution_empirical.hpp"
//#include "core/distribution_shifted_lognormal.hpp"
//#include "core/histogram.hpp"
//#include "core/population_mover_multidim.hpp"
//#include "core/rng_impl.hpp"
//
//using namespace averisera;
//
//TEST(PopulationMoverMultidim, Simple) {
//	const size_t popsize = 20;
//	const size_t NX = 2;
//	const size_t NY = 2;
//	const size_t N = NX * NY;
//	const std::vector<double> ranges1d({ 0, 0.5, 1 });
//	const std::vector<std::vector<double>> ranges({ ranges1d, ranges1d });
//	typedef PopulationMoverMultidim::Member pop_type;
//	std::vector <pop_type> population(popsize);
//	RNGImpl rng(42);
//	for (size_t i = 0; i < popsize; ++i) {
//		const double x = rng.next_uniform();
//		const double y = rng.next_uniform();
//		const size_t kx = x < ranges1d[1] ? 0 : 1;
//		const size_t ky = y < ranges1d[1] ? 0 : 1;
//		const size_t k = kx * NY + ky;
//		population[i].member_index = i;
//		population[i].values = { x, y };
//		population[i].distribution_index = k;		
//	}
//	Eigen::MatrixXd pi(N, N);
//	//// if kx != ky, swap them with 50% probability
//	//// if kx == ky, set kx to 0 and ky to 1 (i.e. k to 1)
//	//pi << 0, 1, 0, 0, // 1st col
//	//	0, 0.5, 0.5, 0, // 2nd col
//	//	0, 1, 0, 0,
//	//	0, 0.5, 0.5, 0;
//	//pi.transposeInPlace(); // rows -> cols
//	pi.setIdentity();
//	PopulationMoverMultidim pm(pi, ranges, true);
//	pm.move_between_ranges(population, rng);
//	ASSERT_EQ(popsize, population.size());
//	std::vector < std::vector<RunningMean<double>>> counters(N);
//	for (auto& col : counters) {
//		col.resize(N);
//	}
//	for (const auto& p : population) {
//		const double x = p.values[0];
//		const double y = p.values[1];
//		const size_t rng_idxX = x < ranges1d[1] ? 0 : 1; // std::min(NX - 1, SegmentSearch::binary_search_left_inclusive(ranges1d, x));
//		const size_t rng_idxY = y < ranges1d[1] ? 0 : 1; // std::min(NY - 1, SegmentSearch::binary_search_left_inclusive(ranges1d, y));
//		const size_t rng_idx = rng_idxX * NY + rng_idxY;
//		const size_t old_rng_idx = p.distribution_index;
//		for (size_t r = 0; r < N; ++r) {
//			counters[old_rng_idx][r].add((r == rng_idx) ? 1.0 : 0.0);
//		}
//	}
//	Eigen::MatrixXd pi_act(N, N);
//	for (size_t c = 0; c < N; ++c) {
//		for (size_t r = 0; r < N; ++r) {
//			pi_act(r, c) = counters[c][r].mean();
//		}
//	}
//	ASSERT_NEAR(0.0, (pi - pi_act).norm(), 1e-6) << pi_act << " vs\n\n" << pi;
//}
//
//TEST(PopulationMoverMultidim, JustX) {
//	const size_t popsize = 20000;
//	const size_t NX = 2;
//	const size_t NY = 1;
//	const size_t N = NX * NY;
//	const std::vector<double> rangesX({ 0, 0.5, 1 });
//	const std::vector<double> rangesY({ 0, 1 });
//	const std::vector<std::vector<double>> ranges({ rangesX, rangesY });
//	typedef PopulationMoverMultidim::Member pop_type;
//	std::vector <pop_type> population(popsize);
//	RNGImpl rng(42);
//	for (size_t i = 0; i < popsize; ++i) {
//		const double x = rng.next_uniform();
//		const double y = rng.next_uniform();
//		const size_t kx = x < rangesX[1] ? 0 : 1;
//		const size_t ky = 0;
//		const size_t k = kx * NY + ky;
//		population[i].member_index = i;
//		population[i].values = { x, y };
//		population[i].distribution_index = k;
//	}
//	Eigen::MatrixXd pi(N, N);
//	// if kx != ky, swap them with 50% probability
//	// if kx == ky, set kx to 1 and ky to 0 (i.e. k to 1)
//	pi << 0, 1, // 1st col
//		0.5, 0.5; // 2nd col
//	pi.transposeInPlace(); // rows -> cols
//	//pi.setIdentity();
//	PopulationMoverMultidim pm(pi, ranges, false);
//	pm.move_between_ranges(population, rng);
//	ASSERT_EQ(popsize, population.size());
//	std::vector < std::vector<RunningMean<double>>> counters(N);
//	for (auto& col : counters) {
//		col.resize(N);
//	}
//	for (const auto& p : population) {
//		const double x = p.values[0];
//		const double y = p.values[1];
//		const size_t rng_idxX = x < rangesX[1] ? 0 : 1;
//		const size_t rng_idxY = 0;
//		const size_t rng_idx = rng_idxX * NY + rng_idxY;
//		const size_t old_rng_idx = p.distribution_index;
//		for (size_t r = 0; r < N; ++r) {
//			counters[old_rng_idx][r].add((r == rng_idx) ? 1.0 : 0.0);
//		}
//	}
//	Eigen::MatrixXd pi_act(N, N);
//	for (size_t c = 0; c < N; ++c) {
//		for (size_t r = 0; r < N; ++r) {
//			pi_act(r, c) = counters[c][r].mean();
//		}
//	}
//	ASSERT_NEAR(0.0, (pi - pi_act).norm(), 1e-2) << pi_act << " vs\n\n" << pi;
//}
//
//
//TEST(PopulationMoverMultidim, JustY) {
//	const size_t popsize = 20000;
//	const size_t NX = 1;
//	const size_t NY = 2;
//	const size_t N = NX * NY;
//	const std::vector<double> rangesY({ 0, 0.5, 1 });
//	const std::vector<double> rangesX({ 0, 1 });
//	const std::vector<std::vector<double>> ranges({ rangesX, rangesY });
//	typedef PopulationMoverMultidim::Member pop_type;
//	std::vector <pop_type> population(popsize);
//	RNGImpl rng(42);
//	for (size_t i = 0; i < popsize; ++i) {
//		const double x = rng.next_uniform();
//		const double y = rng.next_uniform();
//		const size_t ky = y < rangesY[1] ? 0 : 1;
//		const size_t kx = 0;
//		const size_t k = kx * NY + ky;
//		population[i].member_index = i;
//		population[i].values = { x, y };
//		population[i].distribution_index = k;
//	}
//	Eigen::MatrixXd pi(N, N);
//	// if kx != ky, swap them with 50% probability
//	// if kx == ky, set kx to 1 and ky to 0 (i.e. k to 1)
//	pi << 0, 1, // 1st col
//		0.5, 0.5; // 2nd col
//	pi.transposeInPlace(); // rows -> cols
//	PopulationMoverMultidim pm(pi, ranges, false);
//	pm.move_between_ranges(population, rng);
//	ASSERT_EQ(popsize, population.size());
//	std::vector < std::vector<RunningMean<double>>> counters(N);
//	for (auto& col : counters) {
//		col.resize(N);
//	}
//	for (const auto& p : population) {
//		const double x = p.values[0];
//		const double y = p.values[1];
//		const size_t rng_idxY = y < rangesY[1] ? 0 : 1;
//		const size_t rng_idxX = 0;
//		const size_t rng_idx = rng_idxX * NY + rng_idxY;
//		const size_t old_rng_idx = p.distribution_index;
//		for (size_t r = 0; r < N; ++r) {
//			counters[old_rng_idx][r].add((r == rng_idx) ? 1.0 : 0.0);
//		}
//	}
//	Eigen::MatrixXd pi_act(N, N);
//	for (size_t c = 0; c < N; ++c) {
//		for (size_t r = 0; r < N; ++r) {
//			pi_act(r, c) = counters[c][r].mean();
//		}
//	}
//	ASSERT_NEAR(0.0, (pi - pi_act).norm(), 5e-3) << pi_act << " vs\n\n" << pi;
//}
//

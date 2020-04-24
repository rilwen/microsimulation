#include <gtest/gtest.h>
#include "core/distribution_empirical.hpp"
#include "core/distribution_shifted_lognormal.hpp"
#include "core/histogram.hpp"
#include "core/population_mover.hpp"
#include "core/rng_impl.hpp"
#include "core/running_mean.hpp"
#include "core/stl_utils.hpp"
#include "testing/rng_precalc.hpp"

namespace averisera {
	typedef std::pair<size_t, double> pop_type;

	// :: for absolute reference
	class PopulationMoverTest : public ::testing::Test
	{
	protected:
		static std::vector<size_t> get_range_indices(PopulationMover& pm, const std::vector<pop_type>& population) {
			return pm.get_range_indices(population);
		}
	};

	using namespace averisera::testing;

	TEST_F(PopulationMoverTest, get_range_indices) {
		const std::vector<pop_type> population({
			{100, 5.1},
			{50, 7.3},
			{10, 8.3},
			{200, 12.3},
			{201, 20.1},
			{34, 22.1}
		});
		const std::vector<double> ranges({ 0., 5., 10., 15., 20., 25. });
		PopulationMover pm(Eigen::MatrixXd(ranges.size() - 1, ranges.size() - 1), ranges);
		const std::vector<size_t> range_indices(get_range_indices(pm, population));
		ASSERT_EQ(std::vector<size_t>({ 0, 0, 3, 4, 4, 6 }), range_indices);
	}

	TEST_F(PopulationMoverTest, get_range_indices2) {
		const std::vector<pop_type> population({
			{ 100, 0.0 },
			{ 50, 7.3 },
			{ 10, 8.3 },
			{ 200, 12.3 },
			{ 201, 20.1 },
			{ 34, 25.0 }
		});
		const std::vector<double> ranges({ 0., 5., 10., 15., 20., 25. });
		PopulationMover pm(Eigen::MatrixXd(ranges.size() - 1, ranges.size() - 1), ranges);
		const std::vector<size_t> range_indices(get_range_indices(pm, population));
		ASSERT_EQ(std::vector<size_t>({ 0, 1, 3, 4, 4, 6 }), range_indices);
	}

	/*TEST_F(PopulationMoverTest, approximate_linear_pdf) {
		size_t n = 100;
		double sum = 0;
		double dx = 1.0 / static_cast<double>(n);
		for (size_t i = 0; i < n; ++i) {
			sum += PopulationMover::approximate_linear_pdf(i * dx, dx) * dx;
		}
		ASSERT_NEAR(1.0, sum, 1E-13);
		sum = 0;
		n = 100000000;
		dx = 1.0 / static_cast<double>(n);
		for (size_t i = 0; i < n; ++i) {
			sum += PopulationMover::approximate_linear_pdf(i * dx, dx) * dx;
		}
		ASSERT_NEAR(1.0, sum, 1E-13);
	}*/

	TEST_F(PopulationMoverTest, get_range_indices3) {
		const std::vector<pop_type> population({
			{ 100, 5.1 },
			{ 50, 7.3 },
			{ 10, 8.3 },
			{ 200, 12.3 },
			{ 201, 25 },
			{ 34, 25 }
		});
		const std::vector<double> ranges({ 0., 5., 10., 15., 20., 25. });
		PopulationMover pm(Eigen::MatrixXd(ranges.size() - 1, ranges.size() - 1), ranges);
		const std::vector<size_t> range_indices(get_range_indices(pm, population));
		ASSERT_EQ(std::vector<size_t>({ 0, 0, 3, 4, 4, 6 }), range_indices);
	}

	TEST_F(PopulationMoverTest, get_range_indices4) {
		const std::vector<pop_type> population({
			{ 100, 5.0 }, // belongs to range 1 (i.e. the 2nd one)
		});
		const std::vector<double> ranges({ 0., 5., 10., 15., 20., 25. });
		PopulationMover pm(Eigen::MatrixXd(ranges.size() - 1, ranges.size() - 1), ranges);
		const std::vector<size_t> range_indices(get_range_indices(pm, population));
		ASSERT_EQ(std::vector<size_t>({ 0, 0, 1, 1, 1, 1 }), range_indices);
	}

	TEST_F(PopulationMoverTest, draw_moved_indices) {
		const size_t N = 3;
		Eigen::VectorXd distr(N);
		distr << 0.25, 0.5, 0.25;
		const size_t i0 = 2;
		const size_t from_size = 16;
		const size_t from = 1;
		std::vector<std::vector<size_t>> ti(N);
		RNGPrecalc rng({ 0.125, 0.375, 0.625, 0.875,
			0.125, 0.375, 0.625, 0.875,
			0.125, 0.375, 0.625, 0.875,
			0.125, 0.375, 0.625, 0.875 });
		std::vector<double> ld(N);
		PopulationMover::draw_moved_indices(distr, i0, from_size, from, ti, rng, ld);
		ASSERT_EQ(16, ti[0].size() + ti[1].size() + ti[2].size());
		ASSERT_EQ(4, ti[0].size());
		ASSERT_EQ(8, ti[1].size());
		ASSERT_EQ(4, ti[2].size());
		std::vector<double> means(N);
		for (size_t i = 0; i < N; ++i) {
			means[i] = static_cast<double>(std::accumulate(ti[i].begin(), ti[i].end(), size_t(0))) / static_cast<double>(ti[i].size());
		}
		ASSERT_LT(means[0], means[1]);
		ASSERT_LT(means[1], means[2]);
	}

	TEST_F(PopulationMoverTest, move_and_draw_new_values) {
		const std::vector<PopulationMover::Member> population({
                { 1, 1, 5.01, 1 },
                { 2, 1, 5.02, 1 },
                { 3, 1, 5.03, 1 },
                { 4, 1, 5.04, 1 },
                { 5, 1, 5.05, 1 },
                { 6, 1, 5.06, 1 }
		});
		const std::vector<double> ranges({ 0., 5., 10., 15., 20., 25. });
		PopulationMover pm(Eigen::MatrixXd(ranges.size() - 1, ranges.size() - 1), ranges);
		std::vector<size_t> ti({ 0, 1, 4, 5 });
		std::vector<double> new_x_values;
		const size_t from = 1;
		const size_t to = 2;
		std::vector<PopulationMover::Member> new_population;
		RNGImpl rng(42);
		pm.move_and_draw_new_values(ti, new_x_values, from, to, population, new_population, rng);
		RunningMean<double> rm;
		for (const auto& p : new_population) {
			rm.add(p.value);
			ASSERT_EQ(static_cast<size_t>(p.value / 5.0), p.range_index) << p.value << " " << p.range_index;
		}
		ASSERT_LT(rm.mean(), 12.5);
		ASSERT_EQ(ti.size(), new_population.size());
		for (size_t i = 0; i < new_population.size(); ++i) {
			ASSERT_EQ(population[ti[i]].member_index, new_population[i].member_index) << i;
			if (i > 0) {
				ASSERT_LE(new_population[i - 1].value, new_population[i].value) << i;
			}
		}
		//std::cout << new_population << "\n";
	}

	TEST_F(PopulationMoverTest, LargePopulation) {
		const size_t popsize = 20000;
		const double shift = 15;
		const double mu = 2;
		const double sigma = 0.5;
		DistributionShiftedLognormal d1(mu, sigma, shift);
		std::vector<pop_type> population(popsize);
		RNGImpl rng(42);
		std::vector<double> values(popsize);
		const std::vector<double> ranges({ shift, 25, 35, 60 });
		const size_t N = ranges.size() - 1;
		Histogram h1(ranges.front(), ranges.back(), static_cast<size_t>(ranges.back() - ranges.front()));
		for (size_t i = 0; i < popsize; ++i) {
			const double v = std::min(d1.draw(rng), ranges.back());
			const size_t rng_idx = std::min(N - 1, SegmentSearch::binary_search_left_inclusive(ranges, v));
			population[i] = pop_type(rng_idx, v);
			values[i] = v;
			h1.add(v);
		}
		DistributionEmpirical de1(std::move(values));
		//std::cout << ranges << std::endl;
		Eigen::MatrixXd pi(N, N);
		pi.col(0) << 0.7, 0.25, 0.05;
		pi.col(1) << 0.1, 0.8, 0.1;
		pi.col(2) << 0.01, 0.09, 0.9;
		Eigen::VectorXd p0(N);
		for (size_t i = 0; i < N; ++i) {
			p0[i] = de1.range_prob(ranges[i], ranges[i + 1]);
		}
		const Eigen::VectorXd p1_exp(pi * p0);
		PopulationMover pm(pi, ranges);
		pm.move_between_ranges(population, rng);
		ASSERT_EQ(popsize, population.size());
		values.clear();
		values.reserve(popsize);
		Histogram h2(ranges.front(), ranges.back(), static_cast<size_t>(ranges.back() - ranges.front()));
		std::vector < std::vector<RunningMean<double>>> counters(N);
		for (auto& col : counters) {
			col.resize(N);
		}
		for (const auto& p : population) {
			h2.add(p.second);
			values.push_back(p.second);
			const size_t rng_idx = std::min(N, SegmentSearch::binary_search_left_inclusive(ranges, p.second));
			const size_t old_rng_idx = p.first;
			for (size_t r = 0; r < N; ++r) {
				counters[old_rng_idx][r].add(r == rng_idx ? 1.0 : 0.0);
			}
		}
		ASSERT_EQ(0, h2.n_below());
		ASSERT_EQ(0, h2.n_above());
		DistributionEmpirical de2(std::move(values));
		Eigen::VectorXd p1_act(N);
		for (size_t i = 0; i < N; ++i) {
			p1_act[i] = de2.range_prob(ranges[i], ranges[i + 1]);
		}
		ASSERT_NEAR(0.0, (p1_act - p1_exp).norm(), 1e-5) << p1_exp << " vs " << p1_act;
		for (size_t i = 0; i < h1.n_bins(); ++i) {
			const double x = h1.lower() + static_cast<double>(i) * h1.bin_size();
			std::cout << x << "\t" << h1.bins()[i] << "\t" << h2.bins()[i] << "\n";
		}
		Eigen::MatrixXd pi_act(N, N);
		for (size_t c = 0; c < N; ++c) {
			for (size_t r = 0; r < N; ++r) {
				pi_act(r, c) = counters[c][r].mean();
			}
		}
		ASSERT_NEAR(0.0, (pi - pi_act).norm(), 3e-2) << pi_act << " vs\n " << pi;
	}

	TEST_F(PopulationMoverTest, LargeAbstract) {
		const size_t popsize = 100000;
		const size_t N = 2;
		const size_t M = 2;
		std::vector<PopulationMover::Member> population(popsize);
		RNGImpl rng(42);
		for (size_t i = 0; i < popsize; ++i) {
			auto& m = population[i];
			m.distribution_index = i % M;
			m.member_index = i;
			m.value = static_cast<double>(i % N) + 0.5 + 0.2 * rng.next_uniform();
		}
		std::vector<double> ranges(N + 1);
		std::iota(ranges.begin(), ranges.end(), 0.0);
		Eigen::MatrixXd pi(N, M);
		pi << 0.0, 1.0,
			1.0, 0.0;
		PopulationMover pm(pi, ranges);
		pm.move_between_ranges(population, rng);
		ASSERT_EQ(popsize, population.size());
		Eigen::VectorXd p0(M);
		p0 << 0.5, 0.5;
		const Eigen::VectorXd p1(pi * p0);
		Eigen::MatrixXd pi_act(N, M);
		Eigen::VectorXd p1_act(N);
		Eigen::VectorXd p0_act(M);
		pi_act.setZero();
		p1_act.setZero();
		p0_act.setZero();
		for (const auto& m : population) {
			const size_t rng_idx = static_cast<size_t>(m.value);
			++p1_act[rng_idx];
			++p0_act[m.distribution_index];
			++pi_act(rng_idx, m.distribution_index);
		}
		p1_act /= p1_act.sum();
		p0_act /= p0_act.sum();
		for (size_t i = 0; i < M; ++i) {
			ASSERT_NEAR(popsize / M, pi_act.col(i).sum(), 1E-10) << i;
			pi_act.col(i) /= pi_act.col(i).sum();
		}
		EXPECT_NEAR(0.0, (p1 - p1_act).norm(), 2E-5) << p1 << "\nvs\n" << p1_act;
		ASSERT_NEAR(0.0, (p0 - p0_act).norm(), 1E-10) << p0 << "\nvs\n" << p0_act;
		ASSERT_NEAR(0.0, (pi - pi_act).norm(), 3E-5) << pi << "\nvs\n" << pi_act;
	}

	TEST_F(PopulationMoverTest, Simple) {
		const size_t popsize = 20000;
		const size_t N = 2;
		const size_t M = 2;
		std::vector<PopulationMover::Member> population(popsize);
		RNGImpl rng(42);
		std::vector<double> ranges({ 0, 0.5, 1 });
		const auto range_calc = [&ranges](double value) { return value < ranges[1] ? 0 : 1; };
		for (size_t i = 0; i < popsize; ++i) {
			auto& m = population[i];
			m.value = rng.next_uniform();
			m.distribution_index = range_calc(m.value);
			m.member_index = i;			
		}
		Eigen::MatrixXd pi(N, M);
		pi << 0.0, 0.5,
			1.0, 0.5;
		PopulationMover pm(pi, ranges, false);
		pm.move_between_ranges(population, rng);
		ASSERT_EQ(popsize, population.size());
		Eigen::VectorXd p0(M);
		p0 << ranges[1], 1-ranges[1];
		const Eigen::VectorXd p1(pi * p0);
		Eigen::MatrixXd pi_act(N, M);
		Eigen::VectorXd p1_act(N);
		pi_act.setZero();
		p1_act.setZero();
		for (const auto& m : population) {
			const size_t rng_idx = range_calc(m.value);
			ASSERT_EQ(rng_idx, m.range_index) << m.value << " " << m.range_index << " " << rng_idx;
			++p1_act[rng_idx];
			++pi_act(rng_idx, m.distribution_index);
		}
		p1_act /= p1_act.sum();
		for (size_t i = 0; i < M; ++i) {
			pi_act.col(i) /= pi_act.col(i).sum();
		}
		EXPECT_NEAR(0.0, (p1 - p1_act).norm(), 2E-2) << p1 << "\nvs\n" << p1_act;
		ASSERT_NEAR(0.0, (pi - pi_act).norm(), 2E-2) << pi << "\nvs\n" << pi_act;
	}

	TEST_F(PopulationMoverTest, get_range_indices_distrs) {
		const std::vector<size_t> sample({ 0, 0, 0, 1, 1, 3, 4, 4, 4, 4 });
		const std::vector<size_t> ranges({ 0, 1, 2, 3, 4, 5 });
		const std::vector<size_t> indices = PopulationMover::get_range_indices(ranges, sample, [](size_t i) {return i; });
		ASSERT_EQ(std::vector<size_t>({ 0, 3, 5, 5, 6, 10 }), indices);
	}
}

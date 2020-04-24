/*
* (C) Averisera Ltd 2017
*/
#include "microsim-core/stitched_markov_model.hpp"
#include "core/rng_impl.hpp"
#include <gtest/gtest.h>

using namespace averisera;
using namespace averisera::microsim;

TEST(StitchedMarkovModel, Simples) {
	const StitchedMarkovModel<>::state_type dim = 2;
	Eigen::VectorXd p0(dim);
	p0 << 0.4, 0.6;
	std::vector<Eigen::MatrixXd> intra(2, Eigen::MatrixXd(dim, dim));
	intra[0] << 0.9, 0.8,
		0.1, 0.2;
	intra[1] << 0.7, 0.25,
		0.3, 0.75;
	std::vector<Eigen::MatrixXd> inter(1, Eigen::MatrixXd(dim, dim));
	inter[0] << 0.1, 0.85,
		0.9, 0.15;
	std::vector<StitchedMarkovModel<>::time_type> lens({ 10 });
	StitchedMarkovModel<> smm(dim, intra, inter, p0, lens);
	ASSERT_EQ(dim, smm.dim());
	ASSERT_EQ(2, smm.nbr_models());
	ASSERT_EQ(0, smm.draw_initial_state(0.0));
	ASSERT_EQ(1, smm.draw_initial_state(1.0));
	ASSERT_EQ(0, smm.draw_initial_state(0.39));
	ASSERT_EQ(1, smm.draw_initial_state(0.41));

	// inter[0]
	ASSERT_EQ(0, smm.draw_next_state(0, 5, 0.89));
	ASSERT_EQ(1, smm.draw_next_state(0, 5, 0.91));
	ASSERT_EQ(0, smm.draw_next_state(1, 5, 0.79));
	ASSERT_EQ(1, smm.draw_next_state(1, 5, 0.81));

	// inter[1]
	ASSERT_EQ(0, smm.draw_next_state(0, 15, 0.69));
	ASSERT_EQ(1, smm.draw_next_state(0, 15, 0.71));
	ASSERT_EQ(0, smm.draw_next_state(1, 15, 0.24));
	ASSERT_EQ(1, smm.draw_next_state(1, 15, 0.26));

	// intra[0]
	// intra[0] * inter[0] == 0.175, 0.25
	//						  0.825, 0.75
	ASSERT_EQ(0, smm.draw_next_state(0, 9, 0.174));
	ASSERT_EQ(1, smm.draw_next_state(0, 9, 0.176));
	ASSERT_EQ(0, smm.draw_next_state(1, 9, 0.24));
	ASSERT_EQ(1, smm.draw_next_state(1, 9, 0.26));

	ASSERT_EQ(0, smm.cache_size());
	ASSERT_NEAR((p0 - smm.calc_state_distribution(0)).norm(), 0., 1e-12);
	ASSERT_NEAR((intra[0] * p0 - smm.calc_state_distribution(1)).norm(), 0., 1e-12);
	ASSERT_NEAR((intra[0] * intra[0] * p0 - smm.calc_state_distribution(2)).norm(), 0., 1e-12);
	smm.precalculate_state_distributions(2);
	ASSERT_EQ(2, smm.cache_size());
	ASSERT_NEAR((p0 - smm.calc_state_distribution(0)).norm(), 0., 1e-12);
	ASSERT_NEAR((intra[0] * p0 - smm.calc_state_distribution(1)).norm(), 0., 1e-12);
	ASSERT_NEAR((intra[0] * intra[0] * p0 - smm.calc_state_distribution(2)).norm(), 0., 1e-12);
	StitchedMarkovModel<> smm2(std::move(smm));
	ASSERT_EQ(2, smm2.cache_size());
	ASSERT_EQ(dim, smm2.dim());
	ASSERT_EQ(2, smm2.nbr_models());
}

TEST(StitchedMarkovModel, PercentileToPercentile) {
	const size_t dim = 3;
	// 0.1 0.5 0.4
	std::vector<double> prev_cdf({ 0.1, 0.6, 1.0 });
	// 0.2 0.1 0.7
	std::vector<double> next_cdf({ 0.2, 0.3, 1.0 });
	Eigen::MatrixXd pi;
	StitchedMarkovModel<>::percentile_to_percentile(prev_cdf, next_cdf, pi);
	ASSERT_EQ(dim, pi.rows());
	ASSERT_EQ(dim, pi.cols());
	for (size_t k = 0; k < dim; ++k) {
		ASSERT_NEAR(pi.col(k).sum(), 1.0, 1e-8) << pi;
		ASSERT_GE(pi.col(k).minCoeff(), 0.0) << pi;
	}
	ASSERT_EQ(1.0, pi(0, 0)) << pi;
	ASSERT_NEAR(0.2, pi(0, 1), 1e-8) << pi;
	ASSERT_NEAR(0.2, pi(1, 1), 1e-8) << pi;
	ASSERT_NEAR(0.6, pi(2, 1), 1e-8) << pi;
	ASSERT_EQ(1.0, pi(2, 2)) << pi;
	prev_cdf = { 0.5, 0.5, 1.0 }; // 0.5 0 0.5
	StitchedMarkovModel<>::percentile_to_percentile(prev_cdf, next_cdf, pi);
	ASSERT_NEAR(0.4, pi(0, 0), 1e-8) << pi;
	ASSERT_NEAR(0.2, pi(1, 0), 1e-8) << pi;
	ASSERT_NEAR(0.4, pi(2, 0), 1e-8) << pi;
	ASSERT_EQ(1.0, pi(2, 1)) << pi;
	ASSERT_EQ(1.0, pi(2, 2)) << pi;
	next_cdf = { 0.2, 1.0, 1.0 }; // 0.2 0.8 0
	StitchedMarkovModel<>::percentile_to_percentile(prev_cdf, next_cdf, pi);
	ASSERT_NEAR(0.4, pi(0, 0), 1e-8) << pi;
	ASSERT_NEAR(0.6, pi(1, 0), 1e-8) << pi;
	ASSERT_EQ(1.0, pi(1, 1)) << pi;
	ASSERT_EQ(1.0, pi(1, 2)) << pi;
}

TEST(StitchedMarkovModel, Ordinal) {
	const StitchedMarkovModel<>::state_type dim = 2;
	std::vector<Eigen::MatrixXd> intra(2, Eigen::MatrixXd(dim, dim));
	intra[0] << 0.9, 0.8,
		0.1, 0.2;
	intra[1] << 0.7, 0.25,
		0.3, 0.75;
	std::vector<StitchedMarkovModel<>::time_type> lens({ 10 });
	std::vector<Eigen::VectorXd> isd(2, Eigen::VectorXd(dim));
	isd[0] << 0.4, 0.6;
	isd[1] << 1.0, 0;
	StitchedMarkovModel<> smm(StitchedMarkovModel<>::ordinal(dim, intra, isd, lens));
	ASSERT_EQ(dim, smm.dim());
	ASSERT_EQ(2, smm.nbr_models());
	Eigen::VectorXd distr = smm.calc_state_distribution(0);
	ASSERT_NEAR((distr - isd[0]).norm(), 0, 1e-10) << distr;
	distr = smm.calc_state_distribution(1);
	ASSERT_NEAR((distr - intra[0] * isd[0]).norm(), 0, 1e-10) << distr;
	distr = smm.calc_state_distribution(lens[0]);
	ASSERT_NEAR((distr - isd[1]).norm(), 0, 1e-10) << distr;
	distr = smm.calc_state_distribution(lens[0] + 1);
	ASSERT_NEAR((distr - intra[1] * isd[1]).norm(), 0, 1e-10) << distr;
}

TEST(StitchedMarkovModel, DrawNextPercentile) {
	const StitchedMarkovModel<>::state_type dim = 2;
	Eigen::VectorXd p0(dim);
	p0 << 0.4, 0.6;
	std::vector<Eigen::MatrixXd> intra(1, Eigen::MatrixXd(dim, dim));
	intra[0] << 0.9, 0.8,
		0.1, 0.2;
	std::vector<Eigen::MatrixXd> inter;
	std::vector<StitchedMarkovModel<>::time_type> lens;
	const StitchedMarkovModel<> smm(dim, intra, inter, p0, lens);
	const size_t n = 10000;
	std::vector<double> perc0(n);
	std::vector<double> perc1(n);
	std::vector<StitchedMarkovModel<>::state_type> k0(n);
	std::vector<StitchedMarkovModel<>::state_type> k1(n);
	RNGImpl rng(42);
	for (size_t i = 0; i < n; ++i) {
		perc0[i] = rng.next_uniform();
		k0[i] = smm.draw_initial_state(perc0[i]);
		const double u = (static_cast<double>(i) + 0.5) / static_cast<double>(n);
		const auto res = smm.draw_next_state_and_percentile(k0[i], 0, u);
		k1[i] = res.first;
		perc1[i] = res.second;
	}
	const Eigen::VectorXd p1(intra[0] * p0);
	const auto n1_0 = std::count_if(k1.begin(), k1.end(), [](StitchedMarkovModel<>::state_type k) { return k == 0; });
	ASSERT_NEAR(p1[0], static_cast<double>(n1_0) / n, 0.001);
	for (double ul = 0; ul < 0.95; ul += 0.1) {
		const auto dn = std::count_if(perc1.begin(), perc1.end(), [ul](double p) { return (p >= ul) && (p <= ul + 0.1); });
		const double du = static_cast<double>(dn) / n;
		ASSERT_NEAR(0.1, du, 0.01) << ul;
	}
}

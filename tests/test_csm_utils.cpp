// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/csm_utils.hpp"
#include "core/markov.hpp"

TEST(CSMUtils, Reduce) {
	const unsigned int memory = 1;
	const unsigned int dim = 2;
	const unsigned int state_dim = averisera::Markov::calc_state_dim(dim, memory);
	std::vector<double> state = { 0.1, 0.2, 0.3, 0.4 };
	std::vector<double> distr(dim);
	averisera::CSMUtils::reduce(&state[0], state_dim, &distr[0], dim, 0);
	ASSERT_NEAR(0.4, distr[0], 1E-15);
	ASSERT_NEAR(0.6, distr[1], 1E-15);
	averisera::CSMUtils::reduce(&state[0], state_dim, &distr[0], dim, 1);
	ASSERT_NEAR(0.3, distr[0], 1E-15);
	ASSERT_NEAR(0.7, distr[1], 1E-15);
}

TEST(CSMUtils, Reduce2) {
	const unsigned int memory = 2;
	const unsigned int dim = 2;
	const unsigned int state_dim = averisera::Markov::calc_state_dim(dim, memory);
	std::vector<double> state = { 0.04, 0.11, 0.16, 0.21, 0.06, 0.09, 0.14, 0.19 };
	std::vector<double> distr(dim);
	averisera::CSMUtils::reduce(&state[0], state_dim, &distr[0], dim, 0);
	ASSERT_NEAR(0.04 + 0.16 + 0.06 + 0.14, distr[0], 1E-15);
	ASSERT_NEAR(0.11 + 0.21 + 0.09 + 0.19, distr[1], 1E-15);
	averisera::CSMUtils::reduce(&state[0], state_dim, &distr[0], dim, 1);
	ASSERT_NEAR(0.04 + 0.11 + 0.06 + 0.09, distr[0], 1E-15);
	ASSERT_NEAR(0.16 + 0.21 + 0.14 + 0.19, distr[1], 1E-15);
	averisera::CSMUtils::reduce(&state[0], state_dim, &distr[0], dim, 2);
	ASSERT_NEAR(0.04 + 0.11 + 0.16 + 0.21, distr[0], 1E-15);
	ASSERT_NEAR(0.06 + 0.09 + 0.14 + 0.19, distr[1], 1E-15);

	// Distribution of X_t and X_{t-1}
	std::vector<double> distr2(dim * dim);
	averisera::CSMUtils::reduce(&state[0], state_dim, &distr2[0], dim * dim, 0);
	ASSERT_NEAR(0.1, distr2[0], 1E-15);
	ASSERT_NEAR(0.2, distr2[1], 1E-15);
	ASSERT_NEAR(0.3, distr2[2], 1E-15);
	ASSERT_NEAR(0.4, distr2[3], 1E-15);

	averisera::CSMUtils::reduce(&distr2[0], dim * dim, &distr[0], dim, 0);
	ASSERT_NEAR(0.4, distr[0], 1E-15);
	ASSERT_NEAR(0.6, distr[1], 1E-15);
}

TEST(CSMUtils, NormalizeDistributions) {
	std::vector<double> x = { 1, 1, 2, 3, 0, 2 };
	const double result = averisera::CSMUtils::normalize_distributions(x, 2, 2);
	ASSERT_NEAR(0.5, x[0], 1E-15);
	ASSERT_NEAR(0.5, x[1], 1E-15);
	ASSERT_NEAR(0.4, x[2], 1E-15);
	ASSERT_NEAR(0.6, x[3], 1E-15);
	ASSERT_NEAR(0.0, x[4], 1E-15);
	ASSERT_NEAR(1.0, x[5], 1E-15);
	ASSERT_NEAR(1 + 4 * 4 + 1, result, 1E-15);
}

TEST(CSMUtils, ExtrapolateBackwards) {
	const double t0 = 2000;
	const std::vector<double> et = { 2001, 1999, 1998, 2002, 2003, 2000 };
	const unsigned int dim = 3;
	Eigen::MatrixXd pi(dim, dim);
	pi.col(0) = Eigen::Vector3d(0.8, 0.1, 0.1);
	pi.col(1) = Eigen::Vector3d(0.05, 0.9, 0.05);
	pi.col(2) = Eigen::Vector3d(0.25, 0.15, 0.6);
	Eigen::VectorXd init_state = Eigen::Vector3d(0.2, 0.3, 0.5);
	Eigen::MatrixXd ep(dim, et.size());
	averisera::CSMUtils::extrapolate(pi, init_state, t0, et, ep);
	for (size_t i = 0; i < et.size(); ++i) {
		ASSERT_NEAR(ep.col(i).sum(), 1.0, 1E-10) << et[i];
		for (unsigned int k = 0; k < dim; ++k) {
			ASSERT_TRUE(ep.col(i)[k] >= 0) << et[i] << " " << k;
		}
	}
	ASSERT_NEAR(0, (ep.col(0) - pi * init_state).norm(), 1E-14);
	ASSERT_NEAR(0, (init_state - pi * ep.col(1)).norm(), 0.02); // errors when extrapolating backwards are much bigger
	ASSERT_NEAR(0, (init_state - pi * pi * ep.col(2)).norm(), 0.2);
	ASSERT_NEAR(0, (ep.col(3) - pi * pi * init_state).norm(), 1E-14);
	ASSERT_NEAR(0, (ep.col(4) - pi * pi * pi * init_state).norm(), 1E-14);
	ASSERT_NEAR(0, (ep.col(5) - init_state).norm(), 1E-14);
}

TEST(CSMUtils, Extrapolate) {
	const size_t dim = 2;
	const size_t n_inputs = 2;
	std::vector<Eigen::MatrixXd> pis(n_inputs, Eigen::MatrixXd(dim, dim));
	pis[0] << 1, 0
		, 0, 1;
	pis[1] << 0, 1
		, 1, 0;
	Eigen::VectorXd init_state(dim);
	init_state << 0.1, 0.9;
	Eigen::VectorXd init_state_swapped(init_state);
	std::swap(init_state_swapped[0], init_state_swapped[1]);
	std::vector<size_t> seg_lengths = { 2, 2 };
	const size_t extrap_T = 6;
	Eigen::MatrixXd extrap_probs(dim, extrap_T);
	averisera::CSMUtils::extrapolate(pis, init_state, seg_lengths, extrap_probs);
	ASSERT_EQ(init_state, extrap_probs.col(0));
	ASSERT_EQ(init_state, extrap_probs.col(1));
	ASSERT_EQ(init_state_swapped, extrap_probs.col(2));
	ASSERT_EQ(init_state, extrap_probs.col(3));
	ASSERT_EQ(init_state_swapped, extrap_probs.col(4));
	ASSERT_EQ(init_state, extrap_probs.col(5));
}

TEST(CSMUtils, ToHierarchicalCompactForm) {
	const std::vector<unsigned int> dims({ 2, 3 });
	const unsigned int tot_dim = dims[0] * dims[1];
	Eigen::MatrixXd pi_compact(tot_dim, tot_dim);
	pi_compact.setZero();
	pi_compact.col(0).setConstant(1.0 / tot_dim);
	pi_compact(0, 1) = 1.0;
	pi_compact.col(2) << 0.05, 0.1, 0.15, 0.2, 0.25, 0.25;
	pi_compact.col(3) << 0.0, 0.1, 0.2, 0.3, 0.4, 0.0;
	pi_compact.col(4) << 0.1, 0.2, 0.3, 0.4, 0.0, 0.0;
	pi_compact.col(5) << 0.5, 0.25, 0.25, 0.0, 0.0, 0.0;
	const auto result = averisera::CSMUtils::to_hierarchical_compact_form(pi_compact, dims, 0);
	ASSERT_EQ(dims.size(), result.size());
	ASSERT_EQ(dims[0], result[0].rows());
	ASSERT_EQ(tot_dim, result[0].cols());
	ASSERT_EQ(dims[1], result[1].rows());
	ASSERT_EQ(tot_dim * dims[0], result[1].cols());
	Eigen::MatrixXd expi0(dims[0], tot_dim);
	expi0.col(0) << 0.5, 0.5;
	expi0.col(1) << 1.0, 0.0;
	expi0.col(2) << 0.3, 0.7;
	expi0.col(3) << 0.3, 0.7;
	expi0.col(4) << 0.6, 0.4;
	expi0.col(5) << 1.0, 0.0;
	ASSERT_NEAR(0.0, (expi0 - result[0]).norm(), 1E-13) << result[0];
	Eigen::MatrixXd expi1(dims[1], tot_dim * dims[0]);
	expi1.col(0) << 1.0 / 3, 1.0 / 3, 1.0 / 3;
	expi1.col(1) << 1.0 / 3, 1.0 / 3, 1.0 / 3;
	expi1.col(2) << 1.0, 0.0, 0.0;
	expi1.col(3) << 1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0;
	expi1.col(4) << 0.05 / 0.3, 0.1 / 0.3, 0.15 / 0.3;
	expi1.col(5) << 0.2 / 0.7, 0.25 / 0.7, 0.25 / 0.7;
	expi1.col(6) << 0.0, 0.1 / 0.3, 0.2 / 0.3;
	expi1.col(7) << 0.3 / 0.7, 0.4 / 0.7, 0.0;
	expi1.col(8) << 0.1 / 0.6, 0.2 / 0.6, 0.3 / 0.6;
	expi1.col(9) << 1.0, 0.0, 0.0;
	expi1.col(10) << 0.5, 0.25, 0.25;
	expi1.col(11) << 1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0;
	ASSERT_NEAR(0.0, (expi1 - result[1]).norm(), 1E-13) << result[1];
	Eigen::MatrixXd pi_compact_mem1(tot_dim, tot_dim * tot_dim);
	for (unsigned int i = 0; i < tot_dim; ++i) {
		pi_compact_mem1.block(0, i * tot_dim, tot_dim, tot_dim) = pi_compact;
	}
	auto result_mem1 = averisera::CSMUtils::to_hierarchical_compact_form(pi_compact_mem1, dims, 1);
	ASSERT_EQ(dims.size(), result_mem1.size());
	ASSERT_EQ(dims[0], result_mem1[0].rows());
	ASSERT_EQ(tot_dim * tot_dim, result_mem1[0].cols());
	ASSERT_EQ(dims[1], result_mem1[1].rows());
	ASSERT_EQ(tot_dim * tot_dim * dims[0], result_mem1[1].cols());
	for (size_t k = 0; k < dims.size(); ++k) {
		ASSERT_EQ(result[k].rows(), result_mem1[k].rows());
		ASSERT_EQ(tot_dim * result[k].cols(), result_mem1[k].cols());
		for (unsigned int i = 0; i < tot_dim; ++i) {
			ASSERT_NEAR(0.0, (result_mem1[k].block(0, i * result[k].cols(), result[k].rows(), result[k].cols()) - result[k]).norm(), 1E-12);
		}
	}
}

TEST(CSMUtils, ToHierarchicalCompactFormIdentity) {
	const std::vector<unsigned int> dims({ 2, 2 });
	const unsigned int tot_dim = dims[0] * dims[1];
	Eigen::MatrixXd pi_compact(tot_dim, tot_dim);
	pi_compact.setIdentity();
	const auto result = averisera::CSMUtils::to_hierarchical_compact_form(pi_compact, dims, 0);
	Eigen::MatrixXd exp_pi0(dims[0], tot_dim);
	exp_pi0 << 1, 1, 0, 0,
		0, 0, 1, 1;
	ASSERT_NEAR(0.0, (exp_pi0 - result[0]).norm(), 1E-14) << result[0];
	Eigen::MatrixXd exp_pi1(tot_dim * dims[0], dims[1]);
	exp_pi1 << 1, 0, // (0, 0, 0)
		0.5, 0.5, // (0, 0, 1)
		0, 1, // (0, 1, 0)
		0.5, 0.5, // (0, 1, 1)
		0.5, 0.5, // (1, 0, 0)
		1, 0, // (1, 0, 1)
		0.5, 0.5, // (1, 1, 0)
		0, 1;// (1, 1, 1)
	exp_pi1.transposeInPlace();
	ASSERT_NEAR(0.0, (exp_pi1 - result[1]).norm(), 1E-14) << result[1];
}

TEST(CSMUtils, IncreaseMemoryLengthInTransitionMatrix) {
	const unsigned int dim = 2;
	Eigen::MatrixXd pilo(dim, dim);
	pilo(0, 0) = 0.2;
	pilo(1, 0) = 0.8;
	pilo(0, 1) = 0.3;
	pilo(1, 1) = 0.7;
	Eigen::MatrixXd pi;
	averisera::CSMUtils::increase_memory_length_in_transition_matrix(pilo, pi);
	ASSERT_EQ(pilo.rows(), pi.rows());
	ASSERT_EQ(pilo.cols() * dim, pi.cols());
	Eigen::MatrixXd expected(dim, dim * dim);
	expected(0, 0) = 0.2;
	expected(1, 0) = 0.8;
	expected(0, 1) = 0.3;
	expected(1, 1) = 0.7;
	expected(0, 2) = 0.2;
	expected(1, 2) = 0.8;
	expected(0, 3) = 0.3;
	expected(1, 3) = 0.7;
	ASSERT_EQ(0.0, (expected - pi).norm());
}
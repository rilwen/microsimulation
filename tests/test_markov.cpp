// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/markov.hpp"

using namespace averisera;

TEST(Markov, CalcNbrPiCoeffs) {
	ASSERT_EQ(9u, Markov::nbr_pi_coeffs(3, 0));
	ASSERT_EQ(27u, Markov::nbr_pi_coeffs(3, 1));
}

TEST(Markov, CalcSteadyState) {
	Eigen::VectorXd ss;
	Eigen::MatrixXd pi(2, 2);
	pi << 0.7, 0.3,
		0.3, 0.7;
	Markov::calc_steady_state(pi, ss);
	ASSERT_EQ(2, ss.size());
	ASSERT_NEAR(0.5, ss[0], 1e-15);
	ASSERT_NEAR(0.5, ss[1], 1e-15);
}

TEST(Markov, CalcStateDim) {
	ASSERT_EQ(2u, Markov::calc_state_dim(2, 0));
	ASSERT_EQ(4u, Markov::calc_state_dim(2, 1));
}

TEST(Markov, CalcMemory) {
	ASSERT_EQ(0, Markov::calc_memory(3, 3));
	ASSERT_EQ(1, Markov::calc_memory(9, 3));
}

TEST(Markov, TransitionMatrix) {
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim);
	const unsigned int T = 11;
	const unsigned int ntraj = 8;
	data.ltrajs.init_rectangular(ntraj, T);
	data.ltimes.init_rectangular(ntraj, T);


	for (unsigned int q = 0; q < ntraj; ++q) {
		for (unsigned int t = 0; t < T; ++t) {
			data.ltimes(q, t) = static_cast<double>(t);
		}
	}

	for (unsigned int q = 0; q < ntraj; ++q) {
		data.ltrajs(q, 0) = (q / dim) % dim;
		data.ltrajs(q, 1) = q % dim;
	}

	const unsigned int memory = 1;
	for (unsigned int q = 0; q < ntraj; ++q) {
		auto prev_prev = data.ltrajs(q, 0);
		auto prev = data.ltrajs(q, 1);
		for (unsigned int t = 2; t < T; ++t) {
			const auto next = prev == prev_prev ? 1 - prev : prev;
			data.ltrajs(q, t) = next;
			prev_prev = prev;
			prev = next;
		}
	}

	Eigen::MatrixXd pi(Markov::transition_matrix(data, dim, memory, false));
	Eigen::MatrixXd expected_pi(dim, dim * dim);
	expected_pi.setZero();
	expected_pi(1, 0) = 1; // (0, 0) -> (0, 1)
	expected_pi(1, 1) = 1; // (0, 1) -> (1, 1)
	expected_pi(0, 2) = 1; // (1, 0) -> (0, 0)
	expected_pi(0, 3) = 1; // (1, 1) -> (1, 0)
	ASSERT_EQ(dim, pi.rows());
	ASSERT_EQ(dim * dim, pi.cols());
	ASSERT_NEAR(0, (expected_pi - pi).norm(), 1E-12);

	pi = Markov::transition_matrix(data, dim, memory, true);
	expected_pi.setZero();

	// First transition is from unknown state, hence the diffusion of probabilities

	expected_pi(1, 2) = 0.05; // (1, 0) -> (0, 1)
	expected_pi(1, 0) = 0.95; // (0, 0) -> (0, 1)

	expected_pi(1, 3) = 0.05; // (1, 1) -> (1, 1)
	expected_pi(1, 1) = 0.95; // (0, 1) -> (1, 1)

	expected_pi(0, 0) = 0.05; // (0, 0) -> (0, 0)
	expected_pi(0, 2) = 0.95; // (1, 0) -> (0, 0)

	expected_pi(0, 1) = 0.05; // (0, 1) -> (1, 0)
	expected_pi(0, 3) = 0.95; // (1, 1) -> (1, 0)
	ASSERT_EQ(dim, pi.rows());
	ASSERT_EQ(dim * dim, pi.cols());
	ASSERT_NEAR(0, (expected_pi - pi).norm(), 1E-12);
}

TEST(Markov, Dof) {
	ASSERT_EQ(3u, Markov::calc_dof(2, 0));
	ASSERT_EQ(18u + 8u, Markov::calc_dof(3, 1));
}

TEST(Markov, ExpandTransitionMatrixNoMemory) {
	std::vector<double> pi({ 0.5, 0.5, 0.9, 0.1 });
	std::vector<double> pi_expanded(4);
	Markov::expand_transition_matrix(&pi[0], &pi_expanded[0], 2, 2);
	for (size_t i = 0; i < 4; ++i) {
		ASSERT_EQ(pi[i], pi_expanded[i]) << i;
	}
}

TEST(Markov, ExpandTransitionMatrixMemory) {
	const unsigned int dim = 2;
	const unsigned int state_dim = 8; // memory == 2
	std::vector<double> pi({
		0.01, 0.99, // (0, 0, 0) ->
		0.1, 0.9, // (0, 0, 1) ->
		0.2, 0.8, // (0, 1, 0) ->
		0.3, 0.7, // (0, 1, 1) ->
		0.4, 0.6, // (1, 0, 0) ->
		0.5, 0.5, // (1, 0, 1) ->
		0.61, 0.39, // (1, 1, 0) ->
		0.71, 0.29 // (1, 1, 1) ->
	});
	std::vector<double> expected_pi_expanded({
		0.01, 0.99, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.1, 0.9, 0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.2, 0.8, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.3, 0.7,
		0.4, 0.6, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.61, 0.39, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.71, 0.29,
	});
	std::vector<double> pi_expanded(64);
	Markov::expand_transition_matrix(&pi[0], &pi_expanded[0], dim, state_dim);
	for (size_t i = 0; i < pi_expanded.size(); ++i) {
		ASSERT_EQ(expected_pi_expanded[i], pi_expanded[i]) << i;
	}
}
// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/markov_process_simulator.hpp"

TEST(MarkovProcessSimulator, Simulation) {
	const unsigned int dim = 3;
	Eigen::MatrixXd pi(dim, dim);
	// Initialize row by row
	pi << 0.8, 0.1, 0.1,
		0.15, 0.6, 0.0,
		0.05, 0.3, 0.9;
	Eigen::VectorXd init_state(dim);
	init_state << 0.01, 0.1, 0.89;
	const unsigned int T = 100;
	averisera::MarkovProcessSimulator simulator(dim, T);
	ASSERT_EQ(T, simulator.path().size());
	ASSERT_EQ(T, simulator.T());
	ASSERT_EQ(dim, simulator.dim());
	simulator.init(pi, init_state);
	simulator.simulate();
	for (unsigned int t = 0; t < T; ++t) {
		//ASSERT_TRUE(simulator.path()[t] >= 0) << t;
		ASSERT_TRUE(simulator.path()[t] < T) << t;
	}
}

TEST(MarkovProcessSimulator, Averaging) {
	const unsigned int dim = 3;
	Eigen::MatrixXd pi(dim, dim);
	// Initialize row by row
	pi << 0.8, 0.1, 0.1,
		0.15, 0.6, 0.0,
		0.05, 0.3, 0.9;
	Eigen::VectorXd init_state(dim);
	init_state << 0.01, 0.1, 0.89;
	const unsigned int T = 10;
	Eigen::MatrixXd expected_probs(dim, T);
	expected_probs.col(0) = init_state;
	for (unsigned int t = 1; t < T; ++t) {
		expected_probs.col(t) = pi * expected_probs.col(t - 1);
	}
	const unsigned int iters = 10000;
	Eigen::MatrixXd simulated_probs(dim, T);
	simulated_probs.setZero();
	averisera::MarkovProcessSimulator simulator(dim, T);
	simulator.init(pi, init_state);
	for (unsigned int i = 0; i < iters; ++i) {
		simulator.simulate();
		for (unsigned int t = 0; t < T; ++t) {
			const unsigned int k = simulator.path()[t];
			simulated_probs(k, t) += 1.0;
		}
	}
	simulated_probs /= iters;
	for (unsigned int k = 0; k < dim; ++k) {
		for (unsigned int t = 0; t < T; ++t) {
			ASSERT_NEAR(expected_probs(k, t), simulated_probs(k, t), 0.01) << k << ", " << t;
		}
	}
}

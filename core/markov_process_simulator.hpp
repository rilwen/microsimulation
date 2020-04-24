#ifndef __AVERISERA_MARKOV_PROCESS_SIMULATOR_H
#define __AVERISERA_MARKOV_PROCESS_SIMULATOR_H

#include <Eigen/Core>
#include <vector>
#include <random>

// TODO: replace it with STL when possible
#include <boost/random/discrete_distribution.hpp>

namespace averisera {
	// Simulates a path of a Markov process
	class MarkovProcessSimulator {
	public:
		// dim: Dimension of the state space of Markov process (e.g. number of BMI ranges or its square for 1 year memory case)
		// T: number of periods to simulate (e.g. number of years)
		MarkovProcessSimulator(unsigned int dim, unsigned int T);

		MarkovProcessSimulator& operator=(const MarkovProcessSimulator&) = delete;

		// Initialise the simulator with given transition matrix (pi_kl = P(X_t = k | X_t-1 = l)) and initial state distribution (P(X_0 = k)) and run the simulation.
		void init(const Eigen::MatrixXd& transition_matrix, const Eigen::VectorXd& init_state);
		void simulate();

		// Return const reference to simulated path. path[t] = X_t
		const std::vector<unsigned int>& path() const { return _path; }

		// Reset the random number generator
		void reset_rng();

		// Reset the random number generator to given seed
		void reset_rng(unsigned int seed);

		// Return state dimension of Markov process
		unsigned int dim() const { return _dim; }

		// Return number of simulated periods; t = 0, ..., T-1
		unsigned int T() const { return _T; }
	private:		
		// Boost class describing discrete distributions
		typedef boost::random::discrete_distribution<> distribution;

		// Template to handle matrix column objects and real vectors in the same way.
		template <class V> void init_distribution(const V& probs, distribution& distr);
	private:
		const unsigned int _dim; // dimension of Markov process
		const unsigned int _T; // nbr of periods

		// Boost objects used to draw randomly new states of the process
		std::vector<distribution> _conditional_transition_distributions;
		distribution _init_state_distribution;

		// Simulated path: _path[t] = X_t
		std::vector<unsigned int> _path;
		std::mt19937 _gen; // Boost RNG
		std::vector<double> _probs; // tmp vector
	};
}

#endif 

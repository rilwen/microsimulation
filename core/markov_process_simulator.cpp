// (C) Averisera Ltd 2014-2020
#include "markov_process_simulator.hpp"
#include <stdexcept>

// Default RNG seed
static const unsigned int SEED = 17;

namespace averisera {
	MarkovProcessSimulator::MarkovProcessSimulator(unsigned int dim, unsigned int T)
		: _dim(dim), _T(T), _conditional_transition_distributions(dim), _path(T), _gen(SEED), _probs(dim)
	{
	}

	void MarkovProcessSimulator::simulate()
	{
		// Draw the initial state
		if (_T > 0) {
			_path[0] = _init_state_distribution(_gen); // In boost, we draw a number from a distribution by passing the RNG object to the distribution object's function call operator
		}

		// Based on the previous state, draw the next state
		for (unsigned int t = 1; t < _T; ++t) {
			// choose the correct conditional distribution for _path[t] based on the value of _path[t - 1]
			_path[t] = _conditional_transition_distributions[_path[t - 1]](_gen);
		}
	}

	void MarkovProcessSimulator::reset_rng()
	{
		// use default value
		reset_rng(SEED);
	}

	void MarkovProcessSimulator::reset_rng(unsigned int seed)
	{
		_gen.seed(seed);
	}

	// Helper function to initialise a boost discrete distribution from data in an Eigen object (matrix column or vector)
	template <class V> 
	void MarkovProcessSimulator::init_distribution(const V& probs, MarkovProcessSimulator::distribution& distr)
	{
		// Boost needs iterators to beginning and end of the container with distribution probabilities, so we copy the probabiilties to std::vector object
		// which can provide such iterators.
		for (unsigned int k = 0; k < dim(); ++k) {
			_probs[k] = probs[k];
		}		
		// Create new distribution object.
		distr = MarkovProcessSimulator::distribution(_probs.begin(), _probs.end());
	}

	void MarkovProcessSimulator::init(const Eigen::MatrixXd& transition_matrix, const Eigen::VectorXd& init_state)
	{
		// Check if dimensions match
		if (transition_matrix.rows() != transition_matrix.cols()) {
			throw std::domain_error("Transition matrix must be square");
		}
		if (static_cast<unsigned int>(transition_matrix.rows()) != dim() || static_cast<unsigned int>(init_state.size()) != dim()) {
			throw std::domain_error("Dimension mismatch");
		}

		// Copy probabilities from Eigen to boost objects
		init_distribution(init_state, _init_state_distribution);
		for (unsigned int k = 0; k < dim(); ++k) {
			// Z_t|Z_t-1=k ~ col(k)
			init_distribution(transition_matrix.col(k), _conditional_transition_distributions[k]);
		}
	}
}

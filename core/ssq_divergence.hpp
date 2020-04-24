/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_SSQ_DIVERGENCE_H
#define __AVERISERA_SSQ_DIVERGENCE_H

#include <Eigen/Core>
#include <vector>

namespace averisera {
	// Calculates sum-of-squares divergence based on a Gaussian expansion of a Dirichlet posterior distribution for observed samples from a multinomial distribution.
	class SSQDivergence {
	public:
		// observed: Observed probability distributions, column by column
		// nbr_surveys: Number of surveys for each observation date
		SSQDivergence(const Eigen::MatrixXd& observed, const Eigen::VectorXd& nbr_surveys);

		// Calculate weighted error
		// year_idx: Index of the year for which we require the error calculation.
		// ns: Number of surveys for this year (ignored)
		// P: Reference to vector-like Eigen structure with the observed probability distribution
		// Q: Reference to vector-like Eigen structure with the approximate probability distribution
		double operator()(size_t year_idx, double ns, Eigen::Ref<const Eigen::VectorXd> P, Eigen::Ref<const Eigen::VectorXd> Q) const;

		// Used for testing: treturn idx-the weight
		const Eigen::MatrixXd& weight(unsigned int idx) const { return _weights[idx]; }

		// Return the weights vector
		const std::vector<Eigen::MatrixXd>& weights() const { return _weights; }

		// Return the dimension of observed probability distributions
		unsigned int dim() const;
	private:
		std::vector<Eigen::MatrixXd> _weights;
	};
}

#endif

#ifndef __AVERISERA_CONFIDENCE_INTERVALS_HPP
#define __AVERISERA_CONFIDENCE_INTERVALS_HPP
/*
(C) Averisera Ltd 2017
*/

#include <vector>
#include <Eigen/Core>

namespace averisera {
	/** Find confidence intervals by sorting the bootstrapped trends according to their Kullback-Leibler distance from the trend fitted to original data.
	*/
	void calc_confidence_intervals_extrap_probs(const double confidence_level, const Eigen::MatrixXd& extrap_probs, const std::vector<Eigen::MatrixXd>& resampl_extrap_probs, Eigen::MatrixXd& lower_prob_bnds, Eigen::MatrixXd& upper_prob_bnds);
}

#endif // __AVERISERA_CONFIDENCE_INTERVALS_HPP

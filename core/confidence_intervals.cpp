/*
(C) Averisera Ltd 2017
*/

#include "confidence_intervals.hpp"
#include "bootstrap.hpp"
#include "sorting.hpp"
#include "preconditions.hpp"
#include "statistics.hpp"

namespace averisera {
	void calc_confidence_intervals_extrap_probs(const double conf_level, const Eigen::MatrixXd& extrap_probs, const std::vector<Eigen::MatrixXd>& resampl_extrap_probs, Eigen::MatrixXd& lower_prob_bnds, Eigen::MatrixXd& upper_prob_bnds) {
		const unsigned int n_boot = static_cast<unsigned int>(resampl_extrap_probs.size());
		const unsigned int dim = static_cast<unsigned int>(extrap_probs.rows());
		check_equals(extrap_probs.rows(), lower_prob_bnds.rows());
		check_equals(extrap_probs.rows(), upper_prob_bnds.rows());

		const unsigned int extrap_len = static_cast<unsigned int>(extrap_probs.cols());
		check_equals(extrap_probs.cols(), lower_prob_bnds.cols());
		check_equals(extrap_probs.cols(), upper_prob_bnds.cols());

		std::vector<Sorting::index_value_pair<double>> indexed_distances(n_boot);

		// Number of trends belonging to the confidence interval.
		const size_t n_trends_in_ci = Bootstrap<>::one_sided(n_boot, conf_level);

		for (unsigned int iter = 0; iter < n_boot; ++iter) { // iterate over resamplings
			double sum = 0;
			for (unsigned int t = 0; t < extrap_len; ++t) { // iterate over years
				const double kl = Statistics::kl_divergence(extrap_probs.col(t), resampl_extrap_probs[iter].col(t));
				sum += kl;
			}
			sum /= extrap_len;
			indexed_distances[iter] = Sorting::index_value_pair<double>(iter, sum);
		}

		// Sort bootstrapped trend indices according to their distance from the original fitted trend
		Sorting::sort_index_value(indexed_distances);

		// For each period t and BMI range k, the highest/lowest probability within the confidence interval 
		// is the highest/lowest probability (k, t) among the trends which closer (according to K-L divergence)
		// to original fitted trend than the p-th percentile, for confidence level p
		for (unsigned int t = 0; t < extrap_len; ++t) {
			for (unsigned int k = 0; k < dim; ++k) { // iterate over Markov process states
													 // find lowest and highest probability among the trends within the confidence interval
				const auto prob_cmp = [t, k, &resampl_extrap_probs](const Sorting::index_value_pair<double>& left, const Sorting::index_value_pair<double>& right) {
					return resampl_extrap_probs[left.first](k, t) < resampl_extrap_probs[right.first](k, t);
				};
				const auto min_prob_iter = std::min_element(indexed_distances.begin(), indexed_distances.begin() + n_trends_in_ci, prob_cmp);
				const auto max_prob_iter = std::max_element(indexed_distances.begin(), indexed_distances.begin() + n_trends_in_ci, prob_cmp);
				// set lower and upper bound
				lower_prob_bnds(k, t) = resampl_extrap_probs[min_prob_iter->first](k, t);
				upper_prob_bnds(k, t) = resampl_extrap_probs[max_prob_iter->first](k, t);
			}
		}
	}

}

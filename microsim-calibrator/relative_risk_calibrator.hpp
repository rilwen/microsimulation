#pragma once
#include <vector>
#include <Eigen/Core>

namespace averisera {
	namespace microsim {
		namespace RelativeRiskCalibrator {
			/*! Given the average risk R in the population, fractions w_i of the population belonging to each group and the relative risks r_i of each group, calculate the "base" risk B such that
			R = B (sum_i w_i r_i + (1 - sum_j w_j))

			\throw std::domain_error If group_fractions.size() != group_relative_risks.size() or R \notin [0, 1]
			\throw DataException If sum_i w_i > 1, sum_i w_i r_i < 0, or solving for 0 <= B <= 1 is not possible
			*/
			double calc_base_risk(double avg_risk, const std::vector<double>& group_fractions, const std::vector<double>& group_relative_risks);

			/*! Each data set in a row.
			\throw std::domain_error If dimensions do not match */
			std::vector<double> calc_base_risks(const std::vector<double>& avg_risks, const Eigen::MatrixXd& group_fractions, const Eigen::MatrixXd& group_relative_risks);
		}
	}
}

#pragma once
/*
(C) Averisera Ltd 2017
*/
#include <Eigen/Core>
#include <vector>

namespace averisera {
	class FactorSelection {
	public:
		virtual ~FactorSelection();

		/** Select the best set of factors from X to explain y 
		@param X n x p matrix (1 row per sample point, 1 column per dimension)
		@param y n-size vector
		@throw std::domain_error If X.rows() != y.size(). If X is empty.
		@return Selected factor indices in ascending order
		*/
		virtual std::vector<size_t> select(const Eigen::Ref<const Eigen::MatrixXd>& X, const Eigen::Ref<const Eigen::VectorXd>& y) const = 0;
	};
}

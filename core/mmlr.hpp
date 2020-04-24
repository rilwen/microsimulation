#pragma once
/*
(C) Averisera Ltd 2017
*/
#include <Eigen/Core>
#include <vector>

namespace averisera {
	/** Multinomial-multivariate logistic regression:

	f_k(X) := 

	F(X) = 1 + sum_{k=0}^{dim-2} exp(a[k] + b[k] * X)
	P_t(y == 0) = 1 / F(X)
	P_t(y == k) = exp(a[k - 1] + b[k - 1] * X) / F(X);    1 <= k < dim

	Where a[k] = scalar and b[k], X - vector of dimension M
	*/
	class MultinomialMultivariateLogisticRegression {
	public:
		MultinomialMultivariateLogisticRegression();

		/**
		Fit model to N samples
		@param X matrix of size M x N
		@param y vector of length N 
		*/
		void fit(const Eigen::MatrixXd& X, const std::vector<unsigned int>& y);

		/** Reset model to empty state (with dim() == 0)*/
		void reset();

		bool empty() const {
			return dim_ == 0;
		}

		/** Dimension D calculated from data, >= 1 after fitting, 0 before fitting */
		unsigned int dim() const {
			return dim_;
		}

		/** Fitted a: vector of length dim - 1 */
		const Eigen::VectorXd& a() const {
			return a_;
		}

		/** Fitted b: matrix with dim-1 rows and M columns */
		const Eigen::MatrixXd& b() const {
			return b_;
		}
	private:
		unsigned int dim_;
		Eigen::VectorXd a_;
		Eigen::MatrixXd b_;
	};
}


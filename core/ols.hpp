#pragma once
/*
(C) Averisera Ltd 2017 
*/
#include <Eigen/Core>
#include <vector>

namespace averisera {
	/** Ordinary Linear Regression */
	class OLS {
	public:
		/**
		@param fit_intercept Whether to fit intercept or assume 0
		*/
		OLS();

		OLS(OLS&& other);

		OLS& operator=(OLS&& other);

		/** Default value: true */
		void fit_intercept(bool value) {
			fit_intercept_ = value;
		}

		/** Default value: false
		*/
		void calculate_prediction(bool value) {
			calculate_prediction_ = value;
		}

		/** Default value: false
		If set to true, sets calculate_prediction to true also.
		*/
		void calculate_residuals(const bool value) {
			calculate_residuals_ = value;
			if (value) {
				calculate_prediction(true);
			}
		}

		/** Default value: false
		If set to true, sets calculate_residuals to true also.
		*/
		void calculate_metrics(const bool value) {
			calculate_metrics_ = value;
			if (value) {
				calculate_residuals(true);
			}
		}

		/**
		Default value: false.
		If set to true, sets calculate_metrics to true also, and calculates the inverse of the result's covariance matrix too.
		*/
		void calculate_coefficient_covariance_matrix(const bool value) {
			calculate_coefficient_covariance_matrix_ = value;
			if (value) {
				calculate_metrics(true);
			}
		}

		///** Whether the model fits the intercept or not */
		//bool fit_intercept() const {
		//	return fit_intercept_;
		//}

		/** Fit a and b to solve X*a + b = y. On exit, set a() to a p-size vector and b() to intercept (if fit_intercept == true) or 0.0. If calculate_residuals == true, residuals() will be set to y - X*a.
		@param X n x p matrix (1 row per sample point, 1 column per dimension)
		@param y n-size vector
		@throw std::domain_error If X.rows() != y.size(). If X is empty.
		*/
		void fit(const Eigen::Ref<const Eigen::MatrixXd>& X, const Eigen::Ref<const Eigen::VectorXd>& y);

		/** Fit a*x = b */
		void fit(const std::vector<double>& x, const std::vector<double>& y);

		const Eigen::VectorXd& a() const {
			return a_;
		}

		double b() const {
			return b_;
		}

		/** Return predicted values after fit */
		const Eigen::VectorXd& prediction() const {
			return pred_;
		}

		/** Return residuals after fit */
		const Eigen::VectorXd& residuals() const {
			return res_;
		}

		/** Sum of squared residuals (SSR) */
		double ssr() const {
			return ssr_;
		}

		/** Total sum of squares (SST) */
		double sst() const {
			return sst_;
		}

		/** R2 coefficient */
		double r2() const {
			return r2_;
		}
		
		/** Adjusted R2 coefficient */
		double adj_r2() const {
			return adj_r2_;
		}

		/** Bayesian Information Criterion */
		double bic() const {
			return bic_;
		}

		/** BIC for an empty model without any factors (but with intercept fitted if there is one) */
		double empty_bic() const {
			return empty_bic_;
		}

		/** Akaike Information Criterion */
		double aic() const {
			return aic_;
		}

		/** Inverse of fitted coefficients covariance matrix. If fit_intercept == true, the free term is the last one. */
		const Eigen::MatrixXd& inverse_result_covariance() const {
			return inverse_result_covariance_;
		}

		/** Fitted coefficients covariance matrix. If fit_intercept == true, the free term is the last one. */
		const Eigen::MatrixXd& result_covariance() const {
			return result_covariance_;
		}

		/** If the model was fitted or not */
		bool empty() const {
			return empty_;
		}

		/** Return true if ols2 is better than ols1 */
		static bool compare_adj_r2(const OLS& ols1, const OLS& ols2) {
			assert(ols2.calculate_metrics_);
			if (!ols1.empty()) {
				assert(ols1.calculate_metrics_);
				return ols2.adj_r2() < ols1.adj_r2();
			} else {
				return ols2.adj_r2() > 0;
			}
		}

		/** 
		Return a functor which returns true of ols2 is better than ols1. Assumes ols1 and ols2 have the same fit_intercept value.
		@param delta Required BIC difference to consider the difference significant 
		*/
		static auto make_bic_comparator(double delta = 6.0) {
			return [delta](const OLS& ols1, const OLS& ols2) -> bool {
				assert(ols1.fit_intercept_ == ols2.fit_intercept_);
				assert(ols2.calculate_metrics_);
				if (!ols1.empty()) {
					assert(ols1.calculate_metrics_);
					return ols2.bic() + delta < ols1.bic();
				} else {
					return ols2.bic() + delta < ols2.empty_bic();
				}
			};
		}

		/** Return a model factory which returns a model useable for factor selection */
		static auto make_model_factory(bool fit_intercept) {
			return [fit_intercept]() -> OLS {
				OLS ols;
				ols.fit_intercept(fit_intercept);
				ols.calculate_metrics(true);
				assert(ols.calculate_residuals_);
				return ols;
			};
		}

		/** Rank i-th factor by how well it fits residuals */
		static double factor_rank_res_adj_r2(const Eigen::Ref<const Eigen::MatrixXd>& X, const Eigen::Ref<const Eigen::VectorXd>& y, const OLS& model, const std::vector<size_t>& factors, size_t i);
	private:
		// Has assignment operator(s) and copy constructors!
		bool empty_;
		bool fit_intercept_;
		bool calculate_prediction_;
		bool calculate_residuals_;
		bool calculate_metrics_;
		bool calculate_coefficient_covariance_matrix_;
		Eigen::VectorXd a_;
		double b_; /**< free term */
		Eigen::VectorXd res_; /**< residuals */
		Eigen::VectorXd pred_; /**< predicted values */
		double ssr_; /**< sum of squared residuals */
		double sst_;
		double r2_;
		double adj_r2_;
		double bic_;
		double empty_bic_;
		double aic_;
		Eigen::MatrixXd inverse_result_covariance_;
		Eigen::MatrixXd result_covariance_;

		Eigen::VectorXd solve(Eigen::MatrixXd& X, const Eigen::Ref<const Eigen::VectorXd>& y);
	};
}

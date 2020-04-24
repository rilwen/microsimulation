/*
(C) Averisera Ltd 2017
*/
#include "factor_selection_bottom_up.hpp"
#include "log.hpp"
#include "ols.hpp"
#include "preconditions.hpp"
#include <Eigen/SVD>

namespace averisera {
	OLS::OLS()
		: empty_(true),
		fit_intercept_(true),
		calculate_prediction_(false),
		calculate_residuals_(false),
		calculate_metrics_(false),
		calculate_coefficient_covariance_matrix_(false)
	{}

	OLS::OLS(OLS&& other)
		: empty_(other.empty_),
		fit_intercept_(other.fit_intercept_),
		calculate_prediction_(other.calculate_prediction_),
		calculate_residuals_(other.calculate_residuals_),
		calculate_metrics_(other.calculate_metrics_),
		calculate_coefficient_covariance_matrix_(other.calculate_coefficient_covariance_matrix_),
		a_(std::move(other.a_)),
		b_(other.b_),
		res_(std::move(other.res_)),
		pred_(std::move(other.pred_)),
		ssr_(other.ssr_),
		sst_(other.sst_),
		r2_(other.r2_),
		adj_r2_(other.adj_r2_),
		bic_(other.bic_),
		empty_bic_(other.empty_bic_),
		aic_(other.aic_),
		inverse_result_covariance_(std::move(other.inverse_result_covariance_)),
		result_covariance_(std::move(other.result_covariance_))
	{}

	OLS& OLS::operator=(OLS&& other) {
		if (this != &other) {
			empty_ = other.empty_;
			fit_intercept_ = other.fit_intercept_;
			calculate_prediction_ = other.calculate_prediction_;
			calculate_residuals_ = other.calculate_residuals_;
			calculate_metrics_ = other.calculate_metrics_;
			calculate_coefficient_covariance_matrix_ = other.calculate_coefficient_covariance_matrix_;
			a_ = std::move(other.a_);
			b_ = other.b_;
			res_ = std::move(other.res_);
			pred_ = std::move(other.pred_);
			ssr_ = other.ssr_;
			sst_ = other.sst_;
			r2_ = other.r2_;
			adj_r2_ = other.adj_r2_;
			bic_ = other.bic_;
			empty_bic_ = other.empty_bic_;
			aic_ = other.aic_;
			inverse_result_covariance_ = std::move(other.inverse_result_covariance_);
			result_covariance_ = std::move(other.result_covariance_);
		}
		return *this;
	}

	void OLS::fit(const Eigen::Ref<const Eigen::MatrixXd>& X, const Eigen::Ref<const Eigen::VectorXd>& y) {
		const auto n = y.size();
		check_equals(X.rows(), n, "OLS::fit: X.rows() != y.size()");
		check_greater<Eigen::Index>(X.size(), 0, "OLS::fit: X.size() > 0");
		Eigen::MatrixXd x(X);
		const auto p = X.cols();
		if (fit_intercept_) {
			// add 1 more column with ones
			x.conservativeResize(x.rows(), x.cols() + 1);
			x.col(X.cols()).setOnes();
		}
		a_ = solve(x, y);
		if (calculate_prediction_) {
			pred_ = x * a_;
		}
		if (calculate_residuals_) {
			assert(calculate_prediction_);
			res_ = y - pred_;
		}
		if (fit_intercept_) {
			b_ = a_[a_.size() - 1];
			a_.conservativeResize(a_.size() - 1);
		} else {
			b_ = 0.0;
		}
		if (calculate_metrics_) {
			assert(calculate_residuals_);
			ssr_ = res_.squaredNorm();
			Eigen::VectorXd tmp(y);
			double k = static_cast<double>(p);
			double empty_k = 0;
			if (fit_intercept_) {
				// see https://online.stat.psu.edu/~ajw13/stat501/SpecialTopics/Reg_thru_origin.pdf
				tmp.array() -= y.mean();
				k += 1.0;
				empty_k += 1.0;
			}
			const double df_e = static_cast<double>(n) - k;
			const double df_t = static_cast<double>(n - 1);
			sst_ = tmp.squaredNorm();
			r2_ = 1 - ssr_ / sst_;
			adj_r2_ = 1 - (ssr_ / df_e) / (sst_ / df_t);
			const double ll = static_cast<double>(n) * std::log(ssr_ / static_cast<double>(n));
			const double empty_ll = static_cast<double>(n) * std::log(sst_ / static_cast<double>(n));
			aic_ = 2 * k + ll;
			const double log_n = std::log(static_cast<double>(n));
			bic_ = k * log_n + ll;
			empty_bic_ = empty_k * log_n + empty_ll;
		}
		if (calculate_coefficient_covariance_matrix_) {
			assert(calculate_metrics_); 
			const auto dim = x.cols();
			inverse_result_covariance_.resize(dim, dim);
			inverse_result_covariance_.noalias() = x.transpose() * x;
			const Eigen::MatrixXd xtx(inverse_result_covariance_);
			result_covariance_ = Eigen::MatrixXd::Identity(dim, dim);
			Eigen::JacobiSVD<Eigen::MatrixXd> svd(xtx, Eigen::ComputeFullU | Eigen::ComputeFullV);
			for (Eigen::Index i = 0; i < dim; ++i) {
				result_covariance_(i, i) = 1 / svd.singularValues()[i];
			}
			result_covariance_ = result_covariance_ * svd.matrixU().transpose();
			result_covariance_ = svd.matrixU() * result_covariance_;			
			const double sigma2 = ssr_ / static_cast<double>(x.rows() - x.cols());
			result_covariance_ *= sigma2;
			inverse_result_covariance_ /= sigma2;
		}
		empty_ = false;
	}

	void OLS::fit(const std::vector<double>& x, const std::vector<double>& y) {
		fit(Eigen::Ref<const Eigen::MatrixXd>(Eigen::Map<const Eigen::MatrixXd>(&x[0], x.size(), 1u)), 
			Eigen::Ref<const Eigen::VectorXd>(Eigen::Map<const Eigen::VectorXd>(&y[0], y.size())));
	}

	Eigen::VectorXd OLS::solve(Eigen::MatrixXd& X, const Eigen::Ref<const Eigen::VectorXd>& y) {
		Eigen::JacobiSVD<Eigen::MatrixXd> svd(X, Eigen::ComputeThinU | Eigen::ComputeThinV);
		return svd.solve(y);
	}

	double OLS::factor_rank_res_adj_r2(const Eigen::Ref<const Eigen::MatrixXd>& X, const Eigen::Ref<const Eigen::VectorXd>& y, const OLS& model, const std::vector<size_t>& /*factors*/, size_t i) {
		OLS ranking_model;
		ranking_model.fit_intercept(model.fit_intercept_);
		ranking_model.calculate_metrics(true);
		if (!model.empty()) {
			assert(model.calculate_residuals_);
			ranking_model.fit(X.col(i), model.residuals());
		} else {
			ranking_model.fit(X.col(i), y);
		}
		const double rank = ranking_model.adj_r2();
		//LOG_TRACE() << "OLS::factor_rank_res_adj_r2: returning " << rank;
		return rank;
	}

	// Instantiate a bottom-up factor selection using OLS and BIC.
	template <> class FactorSelectionBottomUp<decltype(OLS::make_model_factory), decltype(OLS::factor_rank_res_adj_r2), decltype(OLS::make_bic_comparator(6.0))>;
}

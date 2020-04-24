/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include "log.hpp"
#include "mlr.hpp"
#include "observed_discrete_data.hpp"
#include "preconditions.hpp"
#include "nlopt_wrap.hpp"
#include "statistics.hpp"
#include "moore_penrose.hpp"
#include <cmath>
#include <cassert>
#include <iostream>
#include <Eigen/Dense>

namespace averisera {
	MultinomialLogisticRegression::MultinomialLogisticRegression(unsigned int dim)
		: _dim(dim), _dim_m_1(dim - 1)
	{
		check_that(dim > 0, "Dimension positive");
		_a.resize(dim - 1);
		_b.resize(dim - 1);
	}

	MultinomialLogisticRegression::MultinomialLogisticRegression(const std::vector<double>& a, const std::vector<double>& b)
		: _dim(static_cast<unsigned int>(a.size() + 1)), _dim_m_1(_dim - 1), _a(a), _b(b)
	{
		check_equals(a.size(), b.size());
	}

	double MultinomialLogisticRegression::p(const double t, const unsigned int k) const {
		assert( k < _dim );

		// For numerical accuracy, we calculate exp(_a[k-1] + _b[k-1] * t) / (1 + sum_l=0^{dim-2} exp(_a[l]+_b[l]*t)) as
		// 1 / (exp(-_a[k-1]-_b[k-1]*t) + sum_l=0^{dim-2} exp(_a[l]-_a[k-1]+(_b[l]-_b[k-1]*t))

		double a_numerator;
		double b_numerator;
		if (k > 0) {
			a_numerator = _a[k - 1];
			b_numerator = _b[k - 1];
		} else {
			a_numerator = b_numerator = 0;
		}
		double sum = linexp(t, -a_numerator, -b_numerator);
		for (unsigned int l = 0; l < _dim_m_1; ++l) {
			sum += linexp(t, _a[l] - a_numerator, _b[l] - b_numerator);
		}
		return 1 / sum;
	}

	double MultinomialLogisticRegression::ln_p(const double t, const unsigned int k) const {
		assert(k < _dim);

		const double numerator = k > 0 ? (_a[k - 1] + _b[k - 1] * t) : 0;

		double sum = 0;
		for (unsigned int l = 0; l < _dim_m_1; ++l) {
			sum += linexp(t, _a[l], _b[l]);
		}
		const double denominator = log1p(sum);

		return numerator - denominator;
	}

	// Local functions/types which should not be exported from this file
	namespace {	
		// Workspace class used during estimation
		struct Workspace {
			Workspace(MultinomialLogisticRegression& mlr, const std::vector<double>& t, const Eigen::MatrixXd& p, const std::vector<Eigen::MatrixXd>& weights, const Eigen::VectorXd& num_surveys);
			Workspace& operator=(const Workspace&) = delete;

			MultinomialLogisticRegression& mlr;
			const std::vector<double>& t;
			const Eigen::MatrixXd& p;
			const std::vector<Eigen::MatrixXd>& weights;
			const Eigen::VectorXd& num_surveys;
			std::vector<double> tmp_grad;
			std::vector<double> work;
			double p_logp;
		};

		Workspace::Workspace(MultinomialLogisticRegression& new_mlr, const std::vector<double>& new_t, const Eigen::MatrixXd& new_p, const std::vector<Eigen::MatrixXd>& new_weights, const Eigen::VectorXd& new_num_surveys)
			: mlr(new_mlr), t(new_t), p(new_p), weights(new_weights), num_surveys(new_num_surveys), tmp_grad(2 * (new_mlr.dim() - 1)), work(mlr.dim())
		{
			const size_t T = new_t.size();
			p_logp = 0.0;
			if (T == static_cast<size_t>(new_num_surveys.size())) {
				// MLE estimation
				p_logp = -Statistics::shannon(new_p, new_num_surveys);
			}
		}

		void set_params(MultinomialLogisticRegression& mlr, const std::vector<double> &x) {
			const size_t dim_m_1 = mlr.dim() - 1;
			std::copy(x.begin(), x.begin() + dim_m_1, mlr.a().begin());
			std::copy(x.begin() + dim_m_1, x.end(), mlr.b().begin());
		}

		// Function passed to NLopt: SSQ version
		double nlopt_f_ssq(const std::vector<double> &x, std::vector<double> &grad, void* f_data) {
			assert(f_data);
			Workspace& wksp = *static_cast<Workspace*>(f_data);
			MultinomialLogisticRegression& mlr = wksp.mlr;
			set_params(mlr, x);
			std::fill(grad.begin(), grad.end(), 0.0);		
			double ssq = 0;
			const size_t T = wksp.t.size();
			const size_t dim = wksp.mlr.dim();
			const size_t grad_dim = 2*(dim - 1);
			for (unsigned int t = 0; t < T; ++t) {
				if (grad.empty()) {
					ssq += mlr.ssq(wksp.t[t], wksp.p.col(t), wksp.weights[t], wksp.work);
				} else {
					ssq += mlr.ssq_grad(wksp.t[t], wksp.p.col(t), wksp.weights[t], wksp.tmp_grad, wksp.work);
					for (unsigned int k = 0; k < grad_dim; ++k) {
						grad[k] += wksp.tmp_grad[k];
					}					
				}
			}
			return ssq;
		}

		// Function passed to NLopt: MLE version (calculates Kullback-Leibler divergence to obtain positive error norm for minimization)
		double nlopt_f_ll(const std::vector<double> &x, std::vector<double> &grad, void* f_data) {
			assert(f_data);
			Workspace& wksp = *static_cast<Workspace*>(f_data);
			MultinomialLogisticRegression& mlr = wksp.mlr;
			set_params(mlr, x);
			std::fill(grad.begin(), grad.end(), 0.0);
			double ll = 0;
			const size_t T = wksp.t.size();
			const size_t dim = wksp.mlr.dim();
			const size_t grad_dim = 2 * (dim - 1);
			for (unsigned int t = 0; t < T; ++t) {
				if (!grad.empty()) {
					ll -= mlr.log_likelihood_grad(wksp.t[t], wksp.p.col(t), wksp.num_surveys[t], wksp.tmp_grad);
					for (unsigned int k = 0; k < grad_dim; ++k) {
						grad[k] -= wksp.tmp_grad[k];
					}
				}
				else {
					ll -= mlr.log_likelihood(wksp.t[t], wksp.p.col(t), wksp.num_surveys[t]);
				}				
			}
			// add p*ln(p) term to form Kullback-Leibler divergence
			return ll + wksp.p_logp;
		}
	}

	static double estimate_impl(Workspace& wksp, nlopt::vfunc optimized_function) {
		const clock_t time0 = std::clock();
		MultinomialLogisticRegression& mlr = wksp.mlr;
		check_equals(mlr.dim(), static_cast<unsigned int>(wksp.p.rows()));
		check_equals(wksp.t.size(), static_cast<size_t>(wksp.p.cols()));
		const size_t arg_dim = 2 * (mlr.dim() - 1);
		nlopt::opt opt(nlopt::LD_SLSQP, static_cast<unsigned int>(arg_dim));
		opt.set_min_objective(optimized_function, &wksp);
		static const double rel_tol = 1E-14;
		opt.set_ftol_rel(rel_tol); // relative tolerance for function value
		opt.set_xtol_rel(rel_tol); // relative tolerance for optimal point
		opt.set_maxeval(10000);
		double value = 0;
		std::vector<double> x(arg_dim);
		mlr.init_guess(wksp.t, wksp.p, x);
		run_nlopt("MLR", opt, x, value);
		set_params(mlr, x);
		const clock_t time1 = std::clock();
		LOG_TRACE() << "MLR estimation time in ms: " << (static_cast<double>(time1 - time0) * 1000.0) / CLOCKS_PER_SEC << ", return norm " << value;
		return value;
	}

	double MultinomialLogisticRegression::estimate_ssq(MultinomialLogisticRegression& mlr, const std::vector<double>& t, const Eigen::MatrixXd& p, const std::vector<Eigen::MatrixXd>& weights) {		
		Workspace wksp(mlr, t, p, weights, Eigen::VectorXd()); // Vector of survey numbers is not needed
		return estimate_impl(wksp, nlopt_f_ssq);
	}

	double MultinomialLogisticRegression::estimate_ssq(MultinomialLogisticRegression& mlr, const std::vector<double>& t, const Eigen::MatrixXd& p, const Eigen::VectorXd& nbr_surveys) {
		SSQDivergence ssqdivg(p, nbr_surveys);
		return estimate_ssq(mlr, t, p, ssqdivg.weights());
	}

	double MultinomialLogisticRegression::estimate_ll (MultinomialLogisticRegression& mlr, const std::vector<double>& t, const Eigen::MatrixXd& p, const Eigen::VectorXd& nbr_surveys) {
		Workspace wksp(mlr, t, p, std::vector<Eigen::MatrixXd>(), nbr_surveys); // Vector of covariance matrices is not needed
		return estimate_impl(wksp, nlopt_f_ll);
	}

	double MultinomialLogisticRegression::estimate(MultinomialLogisticRegression& mlr, const std::vector<double>& t, const Eigen::MatrixXd& p, const Eigen::VectorXd& nbr_surveys, MultinomialLogisticRegression::EstimationType estimation_type) {
		switch (estimation_type) {
		case EstimationType::SSQ:
			return estimate_ssq(mlr, t, p, nbr_surveys);
		case EstimationType::MLE:
			return estimate_ll(mlr, t, p, nbr_surveys);
		default:
			throw std::logic_error("Enum value not handled correctly");
		}
	}

	// Finds an initial guess for a's and b's using linear regression. Massively improves convergence of the model.
	void MultinomialLogisticRegression::init_guess(const std::vector<double>& t, const Eigen::MatrixXd& p, std::vector<double>& x) const {
		const size_t dim = p.rows();
		const size_t T = p.cols();
		Eigen::MatrixXd X(T, 2);
		X.col(0).setConstant(1.0);
		for (size_t i = 0; i < T; ++i) {
			X(i, 1) = t[i];
		}
		Eigen::Matrix2d XTX; // 2x2 matrix class
		XTX.noalias() = X.transpose() * X;
		const Eigen::Matrix2d invXTX = XTX.inverse();		
		Eigen::Vector2d beta;
		Eigen::VectorXd y(T);
		for (size_t k = 1; k < dim; ++k) {
			for (size_t i = 0; i < T; ++i) {
				const double p0 = p(0, i);
				const double pk = p(k, i);
				if (pk == p0) {
					y[i] = 0;
				} else if (p0 == 0) {
					// Linear regression won't work for any k because we would be fitting log(p[k] / 0) == infinity
					x.assign(x.size(), 0.0); // try the default initial guess and hope for the best
					return;
				} else if (pk == 0) {
					// This k won't work, skip it
					x[k - 1] = 0.0;
					x[dim - 2 + k] = 0.0; // try the default initial guess for this k
					break; // go to next k
				} else {
					const double yi = log(pk / p0);
					y[i] = yi;
				}
			}
			// do linear regression to obtain a[k] and b[k]
			const Eigen::Vector2d XTy = X.transpose() * y;
			beta = invXTX * XTy;
			x[k - 1] = beta[0];
			x[dim - 2 + k] = beta[1];
		}
	}

	static void add_param_inverse_covariance_matrix_element_lower_diagonal(Eigen::MatrixXd& inv_cov, const unsigned int dim_m_1, const unsigned int i, const unsigned int j, double tau, double val_aa) {
		assert(i >= j);
		inv_cov(i, j) += val_aa; // a[i]a[j]
		const unsigned int i2 = i + dim_m_1;
		const unsigned int j2 = j + dim_m_1;
		const double v2 = val_aa * tau;
		assert(i2 >= j2);
		assert(i2 >= j);
		assert(j2 >= i);
		inv_cov(i2, j) += v2; // b[i]a[j]
		if (i != j) {
			inv_cov(j2, i) += v2; // a[j]b[i]
		}
		inv_cov(i2, j2) += v2 * tau; // b[i]b[j]
	}

	void MultinomialLogisticRegression::calc_param_inverse_covariance_matrix(const std::vector<double>& times, const Eigen::VectorXd& nbr_surveys, Eigen::MatrixXd& inv_cov) const {
		const unsigned int cov_dim = 2 * _dim_m_1;
		inv_cov.setZero(cov_dim, cov_dim);
		const size_t T = times.size();
		assert(static_cast<size_t>(nbr_surveys.size()) == T);
		std::vector<double> work(_dim);
		for (size_t t = 0; t < T; ++t) {
			const double n_t = nbr_surveys[t];
			const double tau = times[t];
			p(tau, work);
			for (unsigned int i = 0; i < _dim_m_1; ++i) {
				const double p_ip1 = work[i + 1];
				add_param_inverse_covariance_matrix_element_lower_diagonal(inv_cov, _dim_m_1, i, i, tau, n_t * p_ip1 * (1 - p_ip1));
				for (unsigned int j = 0; j < i; ++j) {
					add_param_inverse_covariance_matrix_element_lower_diagonal(inv_cov, _dim_m_1, i, j, tau, n_t * p_ip1 * work[j + 1]);
				}
			}
		}
		// copy elements to strict upper diagonal		
		for (unsigned int r = 0; r < cov_dim; ++r) {
			for (unsigned int c = 0; c < r; ++c) {
				inv_cov(c, r) = inv_cov(r, c);
			}
		}
	}

	void MultinomialLogisticRegression::calc_param_covariance_matrix(const std::vector<double>& t, const Eigen::VectorXd& nbr_surveys, Eigen::MatrixXd& cov) const {
		Eigen::MatrixXd inv_cov;
		calc_param_inverse_covariance_matrix(t, nbr_surveys, inv_cov);
		MoorePenrose::inverse(inv_cov, 1E-12, cov);
	}

	void MultinomialLogisticRegression::calc_w_covariance_matrix(const Eigen::MatrixXd& param_cov, double tau, Eigen::MatrixXd& w_cov) const {
		w_cov.resize(_dim_m_1, _dim_m_1);
		for (unsigned int i = 0; i < _dim_m_1; ++i) {
			const unsigned int i2 = i + _dim_m_1;
			for (unsigned int j = 0; j <= i; ++j) {
				const unsigned int j2 = j + _dim_m_1;

				// Use Horner algorithm
				double w_ij = param_cov(i2, j2);
				w_ij = w_ij * tau + param_cov(i2, j) + param_cov(i, j2);
				w_ij = w_ij * tau + param_cov(i, j);
				
				w_cov(i, j) = w_cov(j, i) = w_ij;
			}
		}
	}

	MultinomialLogisticRegression::Model::Model(unsigned int dim)
		: _mlr(new MultinomialLogisticRegression(dim)), _estimation_type(EstimationType::MLE)
	{}

	MultinomialLogisticRegression::Model::Model(std::shared_ptr<const SSQDivergence> ssq)
		: _ssq(ssq), _mlr(new MultinomialLogisticRegression(ssq->dim())), _estimation_type(EstimationType::SSQ)
	{
		assert(_ssq);
	}

	double MultinomialLogisticRegression::Model::operator()(const ObservedDiscreteData& data, const std::vector<double>& extrap_times, Eigen::MatrixXd& extrap_probs) const {
		assert(static_cast<unsigned int>(data.probs.rows()) == _mlr->dim());
		const size_t T = data.times.size();
		double error;
		const Eigen::MatrixXd& p = data.probs;
		const Eigen::VectorXd& ns = data.nbr_surveys;
		if (_estimation_type == EstimationType::SSQ) {
			std::vector<Eigen::MatrixXd> current_weights(_ssq->weights());
			assert(T == current_weights.size());
			for (size_t t = 0; t < T; ++t) {
				if (ns[t] == 0) {
					// for crossvalidation to work we need strictly zero weights when there are no observations---while SSQDivergence would still give a nonzero weight in this case
					current_weights[t].setZero();
				}
			}
			error = MultinomialLogisticRegression::estimate_ssq(*_mlr, data.times, p, current_weights);
		}
		else {
			assert(_estimation_type == EstimationType::MLE);
			error = MultinomialLogisticRegression::estimate_ll(*_mlr, data.times, p, ns);
		}
		const size_t newT = extrap_times.size();
		if (newT > 0) {
			assert(newT == static_cast<size_t>(extrap_probs.cols()));
			assert(data.probs.rows() == extrap_probs.rows());
			for (size_t t = 0; t < newT; ++t) {
				auto distr = extrap_probs.col(t);
				_mlr->p(extrap_times[t], distr);
			}
		}
		return error;
	}

	double MultinomialLogisticRegression::Model::operator()(const ObservedDiscreteData& data, const std::vector<double>& extrap_times, Eigen::MatrixXd& extrap_probs, double confidence_level, Eigen::MatrixXd& extrap_probs_lower, Eigen::MatrixXd& extrap_probs_upper) const {
		check_that(_estimation_type == EstimationType::MLE);
		const double error = operator()(data, extrap_times, extrap_probs);
		const size_t newT = extrap_times.size();
		if (newT > 0) {
			assert(newT == static_cast<size_t>(extrap_probs_lower.cols()));
			assert(data.probs.rows() == extrap_probs_lower.rows());
			assert(newT == static_cast<size_t>(extrap_probs_upper.cols()));
			assert(data.probs.rows() == extrap_probs_upper.rows());
			Eigen::MatrixXd cov;
			_mlr->calc_param_covariance_matrix(data.times, data.nbr_surveys, cov);
			Eigen::MatrixXd w_cov;
			for (size_t t = 0; t < newT; ++t) {
				_mlr->calc_w_covariance_matrix(cov, extrap_times[t], w_cov);
				auto lower = extrap_probs_lower.col(t);
				auto upper = extrap_probs_upper.col(t);
				_mlr->calc_confidence_intervals(w_cov, extrap_probs.col(t), confidence_level, lower, upper);
			}
		}
		return error;
	}

}

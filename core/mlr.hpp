/*
(C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_MULTINOMIAL_LOGISTIC_REGRESSION_H
#define __AVERISERA_MULTINOMIAL_LOGISTIC_REGRESSION_H

#include <cmath>
#include <vector>
#include <memory>
#include <Eigen/Core>
#include "ssq_divergence.hpp"
#include "normal_distribution.hpp"
#include "preconditions.hpp"

namespace averisera {	

	struct ObservedDiscreteData;

	/** Multinomial logistic regression:
	F(t) = 1 + sum_{k=0}^{dim-2} exp(a[k] + b[k] * t)
	P_t(X == 0) = 1 / F(t)
	P_t(X == k) = exp(a[k - 1] + b[k - 1] * t) / F(t);    1 <= k < dim
	*/
	class MultinomialLogisticRegression {
	public:
		// MLE: maximum log-likelihood
		// SSQ: sum of squared errors		
		enum class EstimationType { MLE, SSQ };

		// dim: Number of probabilities, larger than 0.
		MultinomialLogisticRegression(unsigned int dim);

		// a.size() == b.size()
		MultinomialLogisticRegression(const std::vector<double>& a, const std::vector<double>& b);

		MultinomialLogisticRegression& operator=(const MultinomialLogisticRegression&) = delete;

		// P(X == k)
		double p(double t, unsigned int k) const;

		// ln P(X == k)
		double ln_p(double t, unsigned int k) const;

		// Fill in a vector-like object distr with extrapolated probability distribution for time t.
		template <class V> void p(double t, V& distr) const;

		// Calculate the sum of squared deviations of extrapolated probabilities from the observed ones at time t.
		// t: Time
		// p_obs: Vector-like object with observed probability distribution (length dim())
		// weights: Matrix-like object with weights for squared errors (dim() x dim()), assumed to be symmetric.
		// work: Vector-like object for temporary results (length >= dim())
		// SSQ = sum_k,l (p(t, k) - p_obs[k]) * (p(t, l) - p_obs[l]) * weights(k, l)
		template <class V1, class M, class V2> double ssq(double t, const V1& p_obs, const M& weights, V2& work) const;

		// Calculate the log-likelihood of observed probabilities.
		// t: Time
		// p_obs: Vector-like object with observed probability distribution (length dim())
		// nbr_surveys: Number of surveys for this time
		// LL = nbr_surveys * sum_k p_obs[k] * ln(p(t, k))
		template <class V1> double log_likelihood(double t, const V1& p_obs, double nbr_surveys) const;

		// Calculate the sum of squared errors (SSQ) and its gradient over a and b with respect to an observed probability distribution p_obs
		// t: Time
		// p_obs: Vector-like object with observed probability distribution (length dim())
		// weights: Matrix-like object with weights for squared errors (dim() x dim()), assumed to be symmetric.
		// grad: Vector-like object for the gradient (length 2*(dim() - 1))
		// work: Vector-like object for temporary results (length >= dim())
		// SSQ = sum_k,l (p(t, k) - p_obs[k]) * (p(t, l) - p_obs[l]) * weights(k, l)
		// At exit:
		// grad[j] = dSSQ/da[j]
		// grad[dim() + j] = dSSQ/db[j]
		// Returns SSQ
		template <class V1, class M, class V2, class V3>  double ssq_grad(double t, const V1& p_obs, const M& weights, V2& grad, V3& work) const;

		// Calculate the log-likelihood of observed probabilities and its gradient over a and b
		// t: Time
		// p_obs: Vector-like object with observed probability distribution (length dim())
		// nbr_surveys: Number of surveys for this time
		// grad: Vector-like object for the gradient (length 2*(dim() - 1))
		// LL = nbr_surveys * sum_k p_obs[k] * ln(p(t, k))
		template <class V1, class V2> double log_likelihood_grad(double t, const V1& p_obs, double nbr_surveys, V2& grad) const;

		// Access the vector of a coefficients
		std::vector<double>& a() { return _a; }

		// Access the vector of a coefficients
		std::vector<double>& b() { return _b; }

		// Access the vector of a coefficients
		const std::vector<double>& a() const { return _a; }

		// Access the vector of a coefficients
		const std::vector<double>& b() const { return _b; }

		// Return the dimension of the model
		unsigned int dim() const { return _dim; }

		// Number of degrees of freedom (a's and b's)
		static unsigned int dof(unsigned int dim) { return 2 * (dim - 1); }

		// Estimate the parameters of MLR using the SSQ method and return the SSQ error.
		// mlr: MLR object
		// t: Vector of times of observation
		// p: Matrix in which i-th column is an observed probability distribution at time t[i]
		// weights: Vector of weight matrices for each time
		static double estimate_ssq(MultinomialLogisticRegression& mlr, const std::vector<double>& t, const Eigen::MatrixXd& p, const std::vector<Eigen::MatrixXd>& weights);

		// Estimate the parameters of MLR using the SSQ method and return the SSQ error.
		// mlr: MLR object
		// t: Vector of times of observation
		// p: Matrix in which i-th column is an observed probability distribution at time t[i]
		// nbr_surveys: Vector of numbers of surveys
		static double estimate(MultinomialLogisticRegression& mlr, const std::vector<double>& t, const Eigen::MatrixXd& p, const Eigen::VectorXd& nbr_surveys, EstimationType estimation_type);

		// Calculate initial guess
		// t: Vector of times of observation
		// p: Matrix in which i-th column is an observed probability distribution at time t[i]
		// x: Target vector for the initial guess (a's followed by b's)
		void init_guess(const std::vector<double>& t, const Eigen::MatrixXd& p, std::vector<double>& x) const;

		// Calculate steady state (distribution for t == infinity)
		// V: vector-like class
		// steady_state: Vector for the calculated state
		template <class V> void calc_steady_state(V& steady_state) const;

		// Calculate dp[k] / da[l]
		// probs: Vector of precalculated probabilities
		// p_k: probs[k]
		// k: Index of the probability
		// l: Index of the a parameter
		template <class V> inline double dp_da(const V& probs, const double p_k, const unsigned int k, const unsigned int l) const {
			const unsigned int lp1 = l + 1;
			if (k > 0) {
				if (lp1 != k) {
					return -p_k * probs[lp1];
				} else {
					return p_k * (1 - p_k);
				}
			} else {
				return -probs[lp1] * p_k;
			}
		}

		// Calculate the inverse covariance matrix of model parameters, based on the quadratic expansion of log-likelihood. Assumes that the model has been already estimated using the MLE method.
		//
		// t: Vector of times of observation
		// nbr_surveys: Vector of numbers of surveys
		// inv_cov: Output matrix, resized if needed.
		void calc_param_inverse_covariance_matrix(const std::vector<double>& t, const Eigen::VectorXd& nbr_surveys, Eigen::MatrixXd& inv_cov) const;

		// Calculate the covariance matrix of model parameters, based on the quadratic expansion of log-likelihood. Assumes that the model has been already estimated using the MLE method.
		//
		// t: Vector of times of observation
		// nbr_surveys: Vector of numbers of surveys
		// cov: Output matrix, resized if needed.
		void calc_param_covariance_matrix(const std::vector<double>& t, const Eigen::VectorXd& nbr_surveys, Eigen::MatrixXd& cov) const;

		// Calculate the covariance matrix of parameters w = a + b * tau from the covariance matrix of a and b.
		void calc_w_covariance_matrix(const Eigen::MatrixXd& param_cov, double tau, Eigen::MatrixXd& w_cov) const;

		// Calculate confidence intervals for extrapolated probabilities
		// w_cov: Covariance matrix of w parameters
		// probs: Extrapolated probabilities for the same time as w_cov (using MLE estimation)
		// confidence_level: Confidence level, e.g. 0.95;
		// lower: Vector for lower confidence intervals
		// upper: Vector for upper confidence intervals
		// V1, V2, V3: Vector-like class
		template <class V1, class V2, class V3> void calc_confidence_intervals(const Eigen::MatrixXd& w_cov, const V1& probs, double confidence_level, V2& lower, V3& upper) const;

		/** Model class used in templated algorithms e.g. cross-validation. */
		class Model {
		public:
			// Initialize for MLE estimation
			// dim: Dimension of the model
			Model(unsigned int dim);

			// Initialize for SSQ estimation using pre-calculated error weights
			// ssq: Shared ptr to error weights collection
			Model(std::shared_ptr<const SSQDivergence> ssq);

			// Estimate model, extrapolate and return estimation error.
			// data: Observed data
			// extrap_times: Years for extrapolation, in ascending order and all >= years[0]
			// extrap_probs: Matrix for extrapolated probability distributions, written in columns.It is expected to have proper dimensions.
			double operator()(const ObservedDiscreteData& data, const std::vector<double>& extrap_times, Eigen::MatrixXd& extrap_probs) const;

			// Estimate model, extrapolate with analytic confidence intervals and return estimation error. Only for MLE estimation.
			// data: Observed data
			// extrap_times: Times for extrapolation, in ascending order and all >= years[0]
			// extrap_probs: Matrix for extrapolated probability distributions, written in columns.It is expected to have proper dimensions.
			// confidence_level: Confidence level for analytic confidence intervals
			// extrap_probs_lower: Lower confidence intervals for extrapolated probabilities
			// extrap_probs_upper: Lower confidence intervals for extrapolated probabilities
			double operator()(const ObservedDiscreteData& data, const std::vector<double>& extrap_times, Eigen::MatrixXd& extrap_probs, double confidence_level, Eigen::MatrixXd& extrap_probs_lower, Eigen::MatrixXd& extrap_probs_upper) const;

			// Number of degrees of freedom
			unsigned int dof() const { return MultinomialLogisticRegression::dof(_mlr->dim()); }

			// Return the MLR object
			const MultinomialLogisticRegression& mlr() const { return *_mlr; }

			const char* name() const {
				return "MLR";
			}
		private:			
			std::shared_ptr<const SSQDivergence> _ssq;
			std::unique_ptr<MultinomialLogisticRegression> _mlr;
			EstimationType _estimation_type;
		};

		//// Functor calculating error norm minimized by the model
		//struct Error {
		//	// year_idx: Index of the year for which we require the error calculation.
		//	// ns: Number of surveys for this year
		//	// P: Reference to vector-like Eigen structure with the observed probability distribution
		//	// Q: Reference to vector-like Eigen structure with the approximate probability distribution
		//	double operator()(double ns, Eigen::Ref<const Eigen::VectorXd> P, Eigen::Ref<const Eigen::VectorXd> Q);
		//};
	private:
		// Calculate exp(a + b*t) in a way which allows taking the limit b -> infinity safely
		static double linexp(double t, double a, double b) {
			if (b != 0) {
				return exp(a + b * t);
			} else {
				// Avoid 0 * infty if we're taking a time limit.
				return exp(a);
			}
		}		

		// Estimate the parameters of MLR using the SSQ method and return the SSQ error.
		// mlr: MLR object
		// t: Vector of times of observation
		// p: Matrix in which i-th column is an observed probability distribution at time t[i]
		// nbr_surveys: Vector of numbers of surveys
		static double estimate_ssq(MultinomialLogisticRegression& mlr, const std::vector<double>& t, const Eigen::MatrixXd& p, const Eigen::VectorXd& nbr_surveys);

		// Estimate the parameters of MLR using the log-likelihood method and return the log-likelihood error.
		// mlr: MLR object
		// t: Vector of times of observation
		// p: Matrix in which i-th column is an observed probability distribution at time t[i]
		// nbr_surveys: Vector of numbers of surveys
		static double estimate_ll(MultinomialLogisticRegression& mlr, const std::vector<double>& t, const Eigen::MatrixXd& p, const Eigen::VectorXd& nbr_surveys);

		const unsigned int _dim;
		const unsigned int _dim_m_1;
		std::vector<double> _a;
		std::vector<double> _b;
	};

	template <class V> void MultinomialLogisticRegression::p(double t, V& distr) const {		
		for (unsigned int k = 0; k < _dim; ++k) {
			distr[k] = p(t, k);
		}
	}

	template <class V> void MultinomialLogisticRegression::calc_steady_state(V& steady_state) const {
		p(std::numeric_limits<double>::infinity(), steady_state);
	}

	template <class V1, class M, class V2> double MultinomialLogisticRegression::ssq(double t, const V1& p_obs, const M& weights, V2& work) const {		
		for (unsigned int k = 0; k < _dim; ++k) {
			work[k] = p(t, k) - p_obs[k];			
		}
		double sum = 0;
		for (unsigned int k = 0; k < _dim; ++k) {
			const double e_k = work[k];
			sum += e_k * e_k * weights(k, k);
			for (unsigned int l = 0; l < k; ++l) {
				sum += 2 * e_k * work[l] * weights(k, l);
			}
		}
		return sum;
	}

	template <class V1, class M, class V2, class V3> double MultinomialLogisticRegression::ssq_grad(double t, const V1& p_obs, const M& weights, V2& grad, V3& work) const {
		double ssq = 0;
		for (unsigned int k = 0; k < _dim; ++k) {			
			work[k] = p(t, k);
		}
		for (size_t l = 0; l < _dim_m_1; ++l) {
			grad[l] = 0.0;
		}
		// Calculate SSQ and dSSQ/da[l]
		for (unsigned int k = 0; k < _dim; ++k) {
			const double p_k = work[k];
			const double e_k = p_k - p_obs[k];
			const double w_kk = weights(k, k);
			ssq += e_k * e_k * w_kk;
			for (unsigned int l = 0; l < _dim_m_1; ++l) {
				grad[l] += 2 * dp_da(work, p_k, k, l) * w_kk * e_k;
			}
			for (unsigned int l = 0; l < k; ++l) {
				const double p_l = work[l];
				const double e_l = p_l - p_obs[l];
				const double w_kl = weights(k, l);
				ssq += 2 * e_k * e_l * w_kl;
				for (unsigned int l2 = 0; l2 < _dim_m_1; ++l2) {
					grad[l2] += 2 * (dp_da(work, p_k, k, l2) * e_l + dp_da(work, p_l, l, l2) * e_k) * w_kl;
				}
			}
		}

		// For every p[k], dp[k]/db[l] = t * dp[k]/da[l]. Hence, dSSQ/db[l] = t * dSSQ/da[l]
		for (unsigned int l = 0; l < _dim_m_1; ++l) {
			grad[_dim_m_1 + l] = t * grad[l];
		}
		return ssq;
	}

	template <class V1> double MultinomialLogisticRegression::log_likelihood(double t, const V1& p_obs, double nbr_surveys) const {
		double sum_ll = 0;
		for (unsigned int k = 0; k < _dim; ++k) {
			sum_ll += p_obs[k] * ln_p(t, k);
		}
		return nbr_surveys * sum_ll;
	}

	template <class V1, class V2> double MultinomialLogisticRegression::log_likelihood_grad(double t, const V1& p_obs, double nbr_surveys, V2& grad) const {
		double sum_ll = 0;
		for (unsigned int k = 0; k < _dim; ++k) {
			const double ln_p_k = ln_p(t, k);
			sum_ll += p_obs[k] * ln_p_k;
			const double p_k = exp(ln_p_k);
			
			if (k > 0) {
				// Calculate dLL/da[k - 1]
				const double dLL_da = nbr_surveys * (p_obs[k] - p_k);
				grad[k - 1] = dLL_da;
				// For every p[k], dp[k]/db[l] = t * dp[k]/da[l]. Hence, dLL/db[l] = t * dLL/da[l]
				grad[k - 1 + _dim_m_1] = dLL_da * t;
			}
		}

		return nbr_surveys * sum_ll;
	}

	template <class V1, class V2, class V3> void MultinomialLogisticRegression::calc_confidence_intervals(const Eigen::MatrixXd& w_cov, const V1& probs, double confidence_level, V2& lower, V3& upper) const {
		check_that(confidence_level <= 1);
		check_that(confidence_level >= 0);
		const double percentile_lower = (1 - confidence_level) / 2;
		const double sigma_scaling_factor = -NormalDistribution::normsinv(percentile_lower);
		for (unsigned int l = 0; l < _dim; ++l) {
			const double p = probs[l];
			if (p > 0) {
				const double log_p = log(p);
				double var = 0.;
				for (unsigned int j1 = 0; j1 < _dim_m_1; ++j1) {
					const Eigen::MatrixXd::ConstColXpr w_cov_j1 = w_cov.col(j1);
					const double v1 = ((j1 + 1 == l) ? 1 : 0) - probs[j1 + 1];
					for (unsigned int j2 = 0; j2 < _dim_m_1; ++j2) {
						const double v2 = ((j2 + 1 == l) ? 1 : 0) - probs[j2 + 1];
						var += w_cov_j1[j2] * v1 * v2; // w_cov is symmetric
					}
				}
				var = std::abs(var); // avoid numerical inaccuracies
				const double sigma = sqrt(var);
				const double delta = sigma_scaling_factor * sigma;
				lower[l] = exp(log_p - delta);
				upper[l] = exp(log_p + delta);
			} else {
				lower[l] = upper[l] = 0;
			}
		}
	}
}

#endif

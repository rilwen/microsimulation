/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_CSM_OBJECTIVE_FUNCTION_H
#define __AVERISERA_CSM_OBJECTIVE_FUNCTION_H

#include <Eigen/Core>
#include <vector>
#include <memory>
#include <cmath>
#include "csm_params.hpp"
#include "statistics.hpp"
#include "jagged_2d_array.hpp"
#include "observed_discrete_data.hpp"

namespace averisera {
	// L: nesting level of autodiff variables
	template <unsigned int L> struct CSMWorkspace;    

	/**
	Calculates the value and gradient of negative log-likelihood of combined cross-sectional and longitudinal data.
	Assumes the data are regularly sampled (no gaps).
	*/
	class CSMObjectiveFunction {
	public:
		/**
		@param data Observed data (observation times are ignored and data are assumed to be equally spaced).
		@param params Model params.
		@throw std::invalid_argument If params.validate() throws. If params.regulariser is not null and is incompatible with the model.
		*/
	    CSMObjectiveFunction(const ObservedDiscreteData& data, const CSMParams& params);

	    CSMObjectiveFunction& operator=(const CSMObjectiveFunction&) = delete;

		/** Calculate the value and the gradient of the objective function f.
		* x: vector of pi coefficients, arranged column by column, followed by the initial state.
		* grad: on exit, vector for the gradient of f, arranged in the same way as x; grad[i] := \partial f / \partial x[i].
		* add_normalization_term: Whether to add normalization penalty term to -log-likelihood.
		* Returns f(x).
		*/
		double value(const std::vector<double>& x, std::vector<double>& grad, bool add_normalization_term) const;

		/** Calculate the value, the gradient and the jacobian of the objective function f.
		* x: vector of pi coefficients, arranged column by column, followed by the initial state.
		* grad: on exit, vector for the gradient of f, arranged in the same way as x; grad[i] := \partial f / \partial x[i].
		* jacobian: on exit, vector for the jacobian of f, jacobian[i * arg_dim + j] = \partial^2 f / \partial x[i] \partial x[j].
		* add_normalization_term: Whether to add normalization penalty term to -log-likelihood.
		* Returns f(x).
		*/
		double value(const std::vector<double>& x, std::vector<double>& grad, std::vector<double>& jacobian, bool add_normalization_term) const;

		/** Return the dimension of the argument vector */
		unsigned int arg_dim() const { return _arg_dim; }

		/// Return the dimension of the Markov process state.
		unsigned int state_dim() const { return _state_dim; }

		/// Return the model parameters.
		const CSMParams& csm_params() const { return _params; }

		/** Return const ref. to measured probabilities. */
		const std::vector<double>& probs() const { return _p; }

		/** Get memory length. */
		unsigned int memory() const { return _params.memory; }

		/** Return total sum of error weights. */
		double sum_weights() const { return _sum_weights; }

		/** Return weights vector. */
		const std::vector<double>& weights() const {
			return _weights;
		}		

		const Jagged2DArray<ObservedDiscreteData::lcidx_t>& trajs() const {
			return _trajs;
		}

		const Jagged2DArray<double>& traj_times() const {
			return _traj_times;
		}

		double min_time() const {
			return _min_time;
		}

		double max_time() const {
			return _max_time;
		}

		/** Calculate the error of approximating distribution P by Q. 
		Scalar: real scalar value type
		nbr_surveys: Number of observations used to construct P
		P: Observed probability distribution
		Q: Approximate probability distribution
		*/
		template <class Scalar> static Scalar error(double nbr_surveys, Eigen::Ref<const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> P, Eigen::Ref<const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> Q);

		/// Extrapolate probabilities and calculate analytic confidence intervals
		/// Output matrices are expected to have correct dimensions
		/// @param solution Optimized parameters of the model
		/// @param confidence_level Confidence level in (0, 1) range
		/// @throw std::invalid_argument If arguments have wrong dimension.
		void extrapolate(const std::vector<double>& solution, const double confidence_level, Eigen::MatrixXd& cov, Eigen::MatrixXd& probs, Eigen::MatrixXd& lower, Eigen::MatrixXd& upper) const;
	private:
		/** Calculate the unweighted error of approximating distribution P by Q.
		dim: Dimension of the prob. distribution
		*/
		template <class ScalarP, class ScalarQ> static ScalarQ unweighted_error(const ScalarP* P, const ScalarQ* Q, const unsigned int dim);

		template <unsigned int L> typename CSMWorkspace<L>::value_t value(CSMWorkspace<L>& wksp, const std::vector<double>& x, std::vector<typename CSMWorkspace<L>::value_t>& grad, bool add_normalization_term) const;

		template <class Scalar> void extrapolate(const std::vector<Scalar>& pi_expanded, const Scalar* q0, const unsigned int T, std::vector<Scalar>& state_distr_approx, std::vector<Scalar>& p_approx) const;

		/** Calculate the covariance matrix of model parameters (e.g. logits).

		The covariance is calculated assuming that the parameters have a joint Gaussian distribution around the fitted value, with log-density given by the quadratic expansion of the log-likelihood function around
		this fitted value.

		@param[in] solution Vector of model parameters.
		@param[in] dpdx Derivatives of model probabilities (from pi or p0) over the parameters. If parameters are chosen to be probabilities themselves, this vector is filled with 1.
		@param[out] Covariance matrix.
		*/
		void calc_param_covariance_matrix(const std::vector<double>& solution, const std::vector<double>& dpdx, Eigen::MatrixXd& cov) const;
	private:
	    const std::vector<double> _p; /** target probability distributions (from data), laid out one after another p_{kt} = _p[t * _params.dim + k] */
	    const std::vector<double> _weights; /** weights of probabilities for each year (from data) equal to nbr_surveys */		
	    const double _sum_weights; /** Sum of weights */
	    const unsigned int _T; /** number of time points */
	    const CSMParams _params;
	    const unsigned int _state_dim; /** dimension of the Markov process state */
		const unsigned int _nbr_pi_coeffs; /** number of important pi coefficients, equal to _state_dim * _params.dim */
		const unsigned int _arg_dim; /** dimension of the function arguments */
		const unsigned int _n_sum_constrs; /** number of sum constraints */

		/** Workspaces isolating the rest of the code from Sacado header files */
		std::shared_ptr<CSMWorkspace<0>> _wksp;

		// Longitudinal trajectories
		const Jagged2DArray<ObservedDiscreteData::lcidx_t> _trajs;
		// Longitudinal trajectory times
		const Jagged2DArray<double> _traj_times;
		// Initial time
		const double _min_time;
		// Maximum time
		const double _max_time;
	};

	template <class Scalar> Scalar CSMObjectiveFunction::error(double nbr_surveys, Eigen::Ref<const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> P, Eigen::Ref<const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> Q) {
		return nbr_surveys * unweighted_error(P.data(), Q.data(), static_cast<unsigned int>(P.size()));
	}


	template <class ScalarP, class ScalarQ> ScalarQ CSMObjectiveFunction::unweighted_error(const ScalarP* P, const ScalarQ* Q, const unsigned int dim) {
		return Statistics::kl_divergence<const ScalarP*, const ScalarQ*, ScalarQ>(P, Q, dim);
	}
}

#endif

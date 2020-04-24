/*
  (C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_CSM_H
#define __AVERISERA_CSM_H

#include <Eigen/Core>
#include <vector>
#include <memory>
#include "csm_params.hpp"
#include "csm_utils.hpp"
#include "nlopt_wrap.hpp"
#include "observed_discrete_data.hpp"

namespace nlopt {
	class opt;
}

namespace averisera {
	class CSMObjectiveFunction;
	struct ObservedDiscreteData;

	/**
	  @brief Main class implementing Cross-Sectional Markov Model.

	  The model can describe any discrete-valued process X_t \in V = [0, ..., D - 1] in discrete time t with finite memory length M 
	  (if M == 0, X_t is Markovian), by lifting it into the space V^{\otimes (M + 1)}, in which the resulting process 
	  Y_t = (X_t, X_{t-1}, ..., X_{t - M}) is Markovian with a constrained transition matrix: Y_{t,i} = Y_{t-1,i-1}.

	  The observed state probability distribution at time t is p_{k,t} = P(X_t = k).
	  
	  The Markovian state probability distribution at time t is q_{l_0, ..., l_M, t} = P(Y_t = (l_0, ..., l_M)) = P(X_t = l_0, ..., X_{t - M} = l_M).

	  We have p_{k, t} = sum_{l_1=0}^{D-1} ... \sum_{l_M=0}^{D-1} q_{k, l_1, ..., l_M, t}.
	  
	  The transition matrix pi_{k, l_0, ..., l_M} = P(X_t = k | Y_{t - 1} = (l_0, ..., l_M)).

	  We have:
	  
	  p_{k, t} = sum_{l_0=0}^{D-1} ... sum_{l_M=0}^{D-1} pi_{k, l_0, ..., l_M} q_{l_0, ..., l_M, t - 1}.

	  q_{l_0, ..., l_M, t} = sum_{l_{M+1}=0}^{D-1} pi_{l_0, l_1, ..., l_{M + 1}} q_{l_1, ..., l_M, l_{M + 1}, t - 1}.

	  We assume the observed data start at t=0. We estimate pi and q_0 (called the ``initial state distribution'') by
	  maximising the log-likelihood of observed data, producing MLE's \hat{pi} and \hat{q_0}.

	  Data storage conventions for Eigen++ objects:

	  1. Transition matrix pi has dimension D x D^{M + 1}, with every column being a D-dimensional probability distribution.
	  2. Initial state distribution q0 is a D^{M + 1}-dimensional probability distribution stored as a vector.
	*/
	class CSM {
	public:
		/** Default stopping conditions. */
		static StoppingConditions get_default_stopping_conditions();

		/** Default optimisation algorithm. */
		static const nlopt::algorithm default_algorithm = nlopt::LD_SLSQP;

		/** Method of initialising the transition matrix (pi) before optimisation. */
		enum class TransitionMatrixInitialisationMethod {

			/** A transition which preserves last state. */
			IDENTITY,

			/** Maximum entropy distributions. */
			MAX_ENTROPY,

			/**
			Calculate an MLE of pi from longitudinal data. Throws an exception if no longitudinal data available. 
			Uses incomplete trajectories too, i.e. sequences of values which do not uniquely identify the transition
			probability.
			*/
			FROM_TRAJECTORIES,

			/**
			Calculate an MLE of pi from longitudinal data. Throws an exception if no longitudinal data available. 
			Uses only complete trajectories (see FROM_TRAJECTORIES).
			*/
			FROM_TRAJECTORIES_COMPLETE_ONLY,

			/** Try to find the best guess for given data (recommended). */
			HEURISTIC
		};

		/** Convert a string representation of the TransitionMatrixInitialisationMethod enum to the enum value.
		@param name Name of the method.
		@throw std::invalid_argument If name is invalid.
		*/
		static TransitionMatrixInitialisationMethod transition_matrix_initialisation_method_from_string(const std::string& name);

		enum class InitialStateDistributionInitialisationMethod {
			/** Maximum entropy initial guess. */
			MAX_ENTROPY,

			/** An guess based on first observed distribution. */
			FROM_DATA
		};

		/** Convert a string representation of the InitialStateDistributionInitialisationMethod enum to the enum value.
		@param name Name of the method.
		@throw std::invalid_argument If name is invalid.
		*/
		static InitialStateDistributionInitialisationMethod initial_state_distribution_initialisation_method_from_string(const std::string& name);

		/** @brief Construct a CSM object providing the data about
		  @param[in] data Observed data
		  @param[in] params Model params
		  @throw DataException If data are incorrect.
		  @throw std::invalid_argument If params.validate() throws.
		  @throw std::out_of_range If params.validate() throws.
		*/
		CSM(const ObservedDiscreteData& data, const CSMParams& params);

		// No copy constructor
		CSM(const CSM&) = delete;

		// No assignment
		CSM& operator=(const CSM&) = delete;

		// Define the destructor in this library to avoid compiler errors in external programs including this header file.
		~CSM();

		/**
		Perform the estimation of transition matrix pi and initial state distribution and store it in provided objects.
		
		@param[in, out] pi Initial guess for transition matrix. Overwritten at exit.
		@param[in, out] q0 Initial guess for initial state distribution. Overwritten at exit.
		@param[out] estimation_info_string If not null, insert some user-readable information about the estimation result into this string.
		@return Return the last value of minimized error norm.
		*/
		double estimate(Eigen::MatrixXd& pi, Eigen::VectorXd& q0, std::string* estimation_info_string = nullptr);

		/**
		Calculate the number of optimised parameters.
		
		@param dim Dimension of the measured distributions (e.g. number of BMI ranges)
		@param memory Memory length
		@return Positive integer.
		*/
		static unsigned int calc_arg_dim(unsigned int dim, unsigned int memory) {
			return CSMUtils::calc_nbr_probs(dim, memory);
		}

		/**
		Extrapolate fitted trends and calculate analytical confidence intervals.
		Even if input probabilities had missing years, extrapolation is done for every year. Assumes that we only have cross-sectional observations.
		
		@param pi Calibrated transition matrix
		@param q0 Calibrated initial state
		@param extrap_probs Matrix for extrapolated probability distributions, written in columns. It is expected to have proper dimensions.
		@param extrap_probs_lower Matrix for lower bounds of extrapolated probability distributions, written in columns. It is expected to have proper dimensions.
		@param extrap_probs_upper Matrix for upper bounds of extrapolated probability distributions, written in columns. It is expected to have proper dimensions.
		@param confidence_level Confidence level in (0, 1)
		@return Covariance matrix of model parameters.
		@throw std::logic_error If model data have trajectories, or there is no cross-sectional data.
		*/
		Eigen::MatrixXd extrapolate_analytic_confidence_intervals(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, Eigen::MatrixXd& extrap_probs, Eigen::MatrixXd& extrap_probs_lower, Eigen::MatrixXd& extrap_probs_upper, double confidence_level) const;

		/**
		Calculate weighted approximation errors, only for years for which we had data. Errors are stored without gaps: if we had data for 1999 and 2001, the error matrix won't
		have any entries for 2000. The errors are either identical or are a low-value limit of the error norm minimized during calibration, and can be used with the
		apply_errors function during bootstrapping. For Kullback-Leibler divergence, the low-value limit is the absolute error.
		
		@param pi Calibrated transition matrix
		@param q0 Calibrated initial state
		@param errors Matrix for approximation errors: k-th column contains the error of fitting the k-th observed distribution.
		*/
		void calc_errors(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, Eigen::MatrixXd& errors) const;		

		/**
		Generate an initial guess for the pi matrix using given method.
		
		@param pi[in, out] Target matrix for initial guess. Resized if needed.
		@param init_method Initialisation method.
		*/
		void calc_initial_guess_pi(Eigen::MatrixXd& pi, TransitionMatrixInitialisationMethod init_method) const;		

		/// Generate an initial guess for the q0 vector using given method.
		/// @param pi[in, out] Target vector for initial guess. Resized if needed.
		/// @param init_method Initialisation method.
		void calc_initial_guess_q0(Eigen::VectorXd& q0, InitialStateDistributionInitialisationMethod init_method) const;

		/** Get the number of model degrees of freedom (independent parameters). */
		unsigned int dof() const {
			return _dof;
		}		

		/** Dimension of the simulated process with memory */
		unsigned int dim() const {
			return _prms.dim;
		}

		/** Model params */
		const CSMParams& params() const {
			return _prms;
		}

		/** Dimension of the Markov state */
		unsigned int state_dim() const {
			return _state_dim;
		}

		/** Number of optimised parameters. */
		unsigned int arg_dim() const {
			return _arg_dim;
		}

		/** Return the norm of the solution.
		@param[out] grad_over_pi Resized to dim x state_dim
		@param[out] grad_over_q0
		@return Norm of the solution
		*/
		double value(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, Eigen::MatrixXd& grad_over_pi, Eigen::VectorXd& grad_over_q0) const;

		/** Return the norm of the solution. Does not calculate the gradient. */
		double value(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0) const;

		/** Return the norm of the solution. Calculate the gradient and the jacobian. */
		double value(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, Eigen::VectorXd& grad, Eigen::MatrixXd& jacobian) const;

		/** Set stopping conditions.
		@throw std::domain_error If some values are inadmissible.
		@return Reference to model object.
		*/
		CSM& set_stopping_conditions(StoppingConditions stopping_conditions) {
			_stopping_conditions = stopping_conditions;
			return *this;
		}

		/** Get the current stopping conditions. */
		const StoppingConditions& get_stopping_conditions() const {
			return _stopping_conditions;
		}

		/** Whether the model data has a longitudinal component. */
		bool has_trajectories() const {
			return _has_trajectories;
		}

		/** Whether the model data has a cross-sectional component. */
		bool has_cross_sectional_data() const {
			return _has_cross_sectional;
		}

		/**
		Set optimisation algorithm.		
		@param algorithm Algorithm constant.
		@return Reference to model object.
		*/
		CSM& set_algorithm(nlopt::algorithm algorithm) {
			_algorithm = algorithm;
			return *this;
		}

		/** Get the current optimisation algorithm. */
		nlopt::algorithm get_algorithm() const {
			return _algorithm;
		}

		/** Represents the CSM model via a unified interface, so that it can be used in templated model selection and confidence interval calculation algorithms. Can be used to apply the same model to different datasets. */
		class Model {
		public:
			/**
			@param params Model params
			@param keep_pi Preserve calculated pi matrix and initial state vector between calls to the model.
			@param pi_init_method (Used if keep_pi = False or during the first call to model calibration) Which transition matrix initialisation method to use.
			@param q0_init_method Which initial state distribution initialisation method to use.
			@param stopping_conditions Stopping conditions for the CSM model calibration.
			@param algorithm Optimisation algorithm.
			*/
			Model(const CSMParams& params, bool keep_pi, TransitionMatrixInitialisationMethod pi_init_method, 
				InitialStateDistributionInitialisationMethod q0_init_method,
				StoppingConditions stopping_conditions, nlopt::algorithm algorithm);

			// No copy constructor
			Model(const Model&) = delete;

			// No assignment
			Model& operator=(const Model&) = delete;

			/** Return value of keep_pi parameter. */
			bool& keep_pi() { return _keep_pi; }

			/** Calibrate the model to observed probabilities and extrapolate the fitted distributions over the requested years, returning the approximation error.

			@param data Observed data
			@param extrap_times Timesfor extrapolation, in ascending order and all >= times[0]. Will be rounded to int
			@param extrap_probs Matrix for extrapolated probability distributions, written in columns. It is expected to have proper dimensions

			@return Total approximation error
			*/
			double operator()(const ObservedDiscreteData& data, const std::vector<double>& extrap_times, Eigen::MatrixXd& extrap_probs);

			/** Calibrate the model to first set of data and return minus log-likelihood of the second set of data (test).

			@param calibration_data Data for calibration
			@param test_data Data for calculating the returned log-likelihood

			@return Pair of <minus log-likelihood of calibration data, minus log-likelihood of test_data>.
			*/
			std::pair<double, double> operator()(const ObservedDiscreteData& calibration_data, const ObservedDiscreteData& test_data);

			/** @return Model name. */
			const char* name() const {
				return "CSM";
			}
		private:
			const CSMParams _prms;
			const unsigned int _state_dim;
			bool _keep_pi;
			TransitionMatrixInitialisationMethod _pi_init_method;
			InitialStateDistributionInitialisationMethod _q0_init_method;
			StoppingConditions _stopping_conditions;
			nlopt::algorithm _algorithm;
			Eigen::MatrixXd _pi;
			Eigen::VectorXd _q0;

			void set_initial_guesses(const CSM& tme);
		};

	private:		

		/// Estimate the model using a chosen optimisation algorithm.
		/// @param opt Chosen algorithm.
		double estimate(nlopt::opt& opt, Eigen::MatrixXd& pi, Eigen::VectorXd& q0, nlopt::result& nlopt_result);

		/** Calculate estimation errors without assuming any data padding.
			@param pi Transition matrix
			@param q0 Initial state
			@param errors Matrix for errors (column by column). Resized as appropriate.
		*/
		void calc_errors_nopad(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, Eigen::MatrixXd& errors) const;		

		static void set_stopping_conditions(nlopt::opt& opt, const StoppingConditions& stopping_conditions);

		/// Generate an initial guess for the pi matrix, trying to find the best guess for given data.
		/// @param pi Target matrix for initial guess. Resized if needed.
		void calc_initial_guess_pi_heuristic(Eigen::MatrixXd& pi) const;

		/// Generate an initial guess for the pi matrix: a transition which preserves last state.
		/// @param pi Target matrix for initial guess. Resized if needed.
		void calc_initial_guess_pi_identity(Eigen::MatrixXd& pi) const;

		/// Generate an initial guess for the pi matrix: use an MLE of transition matrix from longitudinal data.
		/// @param pi Target matrix for initial guess. Resized if needed.
		/// @param use_incomplete_data Use incomplete trajectories.
		/// @throw std::logic_error If there are no longitudinal data.
		void calc_initial_guess_pi_trajectories(Eigen::MatrixXd& pi, bool use_incomplete_data) const;

		/// Generate an initial guess for the pi matrix: conditional distributions maximizing the entropy (transition to every state has the same probability)
		/// @param pi Target matrix for initial guess. Resized if needed.
		void calc_initial_guess_pi_max_entropy(Eigen::MatrixXd& pi) const;

		/// Generate an initial guess for the initial state distribution based on first observed distribution.
		/// @param q0 Target vector for initial guess. Resized if needed.
		void calc_initial_guess_q0_from_data(Eigen::VectorXd& q0) const;

		/// Generate a maximum entropy initial guess for the initial state distribution.
		/// @param q0 Target vector for initial guess. Resized if needed.
		void calc_initial_guess_q0_max_entropy(Eigen::VectorXd& q0) const;
	private:
		const size_t _T; // Number of time periods in the data
		const CSMParams _prms; // Model params
		const unsigned int _state_dim; // Dimension of the Markov state
		const unsigned int _nbr_coeffs_pi; // Number of optimised pi coefficients
		const unsigned int _arg_dim; // Number of optimised parameters
		const unsigned int _dof; // Number of independent optimised parameters
		const unsigned int _n_sum_constr; // Number of sum constraints (probs. add up to 1)
		std::unique_ptr<CSMObjectiveFunction> _f; // Pointer to objective function minimised during estimation.
		std::vector<double> _lb; // lower bounds for probabilities (0): p_kl >= _lb[l * dim + k]
		std::vector<double> _ub; // upper bounds for probabilities (1): p_kl <= _ub[l * dim + k]
		std::vector<double> _constr_tol; // tolerances for equality constraints on probabilities (foreach_l sum_k p_kl = 1)
		std::vector<double> _x; // Temp. vector for the optimised parameters
		size_t _padded_T; // Time dimension of padded data.
		std::vector<size_t> _input_to_padded; // Maps input data indices to padded data indices (see CSM constructor)
		bool _with_padding; // was padding applied to input data?
		const bool _has_trajectories; // data contain longitudinal trajectories
		const bool _has_cross_sectional; // data contain cross-sectional surveys
		const ObservedDiscreteData _data_reduced; // reduced cross-sectional form of input data
		const ObservedDiscreteData& _orig_data; // ref. to original data        
		StoppingConditions _stopping_conditions; // Stopping conditions for the optimiser.
		nlopt::algorithm _algorithm; // NLopt algorithm used.		
	};

	std::ostream& operator<<(std::ostream& os, const CSM::TransitionMatrixInitialisationMethod method);
}

#endif

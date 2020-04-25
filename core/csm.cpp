/*
  (C) Averisera Ltd 2014
*/
#include "csm.hpp"
#include "observed_discrete_data.hpp"
#include "csm_objective_function.hpp"
#include "csm_utils.hpp"
#include "log.hpp"
#include "nlopt_wrap.hpp"
#include "discrete_distribution.hpp"
#include "markov.hpp"
#include "math_utils.hpp"
#include <array>
#include <vector>
#include <cassert>
#include <iostream>
#include <sstream>
#include <cmath>
#include <ctime>
#include <numeric>
//#include <boost/log/trivial.hpp>
#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <boost/format.hpp>

static const double LOWER_BOUND = 1E-14;
static const bool ADD_NORMALISATION_TERM = true;

namespace averisera {
    
    
	CSM::CSM(const ObservedDiscreteData& data, const CSMParams& params)
		: _T(data.probs.cols())
		, _prms(params.with_dim(data))
		, _state_dim(Markov::calc_state_dim(_prms.dim, _prms.memory))
		, _nbr_coeffs_pi(Markov::nbr_pi_coeffs(_prms.dim, _prms.memory))
		, _arg_dim(calc_arg_dim(_prms.dim, _prms.memory))
		, _dof(Markov::calc_dof(_prms.dim, _prms.memory))
		, _n_sum_constr(_state_dim + 1)
		, _lb(_arg_dim, LOWER_BOUND) // not zero because we don't want to take logs of zeros
		, _ub(_arg_dim, 1.0)
		, _constr_tol(_n_sum_constr)
		, _x(_arg_dim)
		, _has_trajectories(ObservedDiscreteData::has_trajectories(data))
		, _has_cross_sectional(_T > 0)
		, _data_reduced(ObservedDiscreteData::to_cross_sectional(data, 1.0))
		, _orig_data(data)
		, _stopping_conditions(get_default_stopping_conditions())
		, _algorithm(default_algorithm)
    {
		params.validate();
		data.validate();
        // Bound transition probabilities between non-neighbouring states to tr_prob_nn.        
        for (unsigned int i = 0; i < _state_dim; ++i) {
			const unsigned int n = i % _prms.dim;
            for (unsigned int m = 0; m < _prms.dim; ++m) {
                if (std::abs(static_cast<int>(m - n)) > 1) {
                    _ub[i*_prms.dim + m] = _prms.tr_prob_nn;
                }
            }
        }
	
        if (data.times.size() != _T) {
            std::stringstream ss;
            ss << "CSM: Expected " << _T << " year numbers but got " << data.times.size();
            throw std::domain_error(ss.str().c_str());
        }
	
        ObservedDiscreteData padded_data(ObservedDiscreteData::pad(data, _input_to_padded));
        _padded_T = padded_data.nbr_surveys.size();
        _f.reset(new CSMObjectiveFunction(padded_data, _prms));
        _with_padding = _padded_T > _T;
    }	

	StoppingConditions CSM::get_default_stopping_conditions() {
		StoppingConditions sc;
		sc.stopval = 1E-12;
		sc.ftol_rel = 1E-12;
		sc.xtol_rel = 1E-6;
		sc.maxeval = 1000;
		sc.maxtime = 60;
		return sc;
	}

	CSM::~CSM()
	{
	}
	static const std::vector<double> empty_vector(0);

	// If the caller passes a non-empty vector grad, we need to calculate the gradient.
	double nlopt_csm_f(const std::vector<double>& x, std::vector<double>& grad, void* f_data) {
		CSMObjectiveFunction* f = static_cast<CSMObjectiveFunction *>(f_data);
		const double val = f->value(x, grad, true);
		//std::cout << "Value: " << val << std::endl;
		return val;
	}

	double CSM::estimate(Eigen::MatrixXd& pi, Eigen::VectorXd& q0, std::string* estimation_info_string)
	{
		if (static_cast<unsigned int>(q0.size()) != _state_dim) {
			throw std::invalid_argument("CSM: Wrong initial state distribution size");
		}
		
		if (static_cast<unsigned int>(pi.rows()) != _prms.dim) {
			throw std::invalid_argument("CSM: Wrong transition matrix row dimension");
		}
		if (static_cast<unsigned int>(pi.cols()) != _state_dim) {
			throw std::invalid_argument("CSM: Wrong transition matrix column dimension");
		}
		const clock_t time0 = std::clock();		
		LOG_DEBUG() << "CSM: using algorithm " << _algorithm << " (" << nlopt::algorithm_name(_algorithm) << ").";
		nlopt::opt opt(_algorithm, _arg_dim);
		if (_algorithm == nlopt::GD_MLSL_LDS || _algorithm == nlopt::GD_MLSL) {
			nlopt::opt local_opt(nlopt::LD_MMA, _arg_dim);
			set_stopping_conditions(local_opt, _stopping_conditions);
			opt.set_local_optimizer(local_opt);
			opt.set_population(_arg_dim);
		}
		nlopt::result nlopt_result;
		const double norm = estimate(opt, pi, q0, nlopt_result);
		const clock_t time1 = std::clock();
		const double estimation_time_ms = (static_cast<double>(time1 - time0) * 1000.0) / CLOCKS_PER_SEC;
		LOG_DEBUG() << "CSM with memory " << _prms.memory << " estimation time in miliseconds: " << estimation_time_ms << ", return norm " << norm;
		if (estimation_info_string) {
			*estimation_info_string = (boost::format("Estimation time [ms] = %g; Optimizer status: %s") % estimation_time_ms % nlopt::retcodestr(nlopt_result)).str();
		}
		return norm;
	}

	// Ensure that the initial guess for the transition matrix coefficients satisfies standard conditions for probability distributions and our bounds
	static void fix_init_guess_pi(std::vector<double>& x, const unsigned int dim, const unsigned int state_dim) {
		const auto end = x.begin() + state_dim * dim;
		// Enforce lower and upper bounds
		for (auto it = x.begin(); it != end; ++it) {
			if (*it < LOWER_BOUND) {
				*it = LOWER_BOUND;
			} else if (*it > 1) {
				*it = 1;
			}
		}

		// Enforce that each distribution sums to 1
		auto it = x.begin();
		for (unsigned int i = 0; i < state_dim; ++i, it += dim) {
			const auto next_it = it + dim;
			const double sum = std::accumulate(it, next_it, 0.0);
			if (std::abs(sum - 1.0) > 1E-12) {
				std::for_each(it, next_it, [sum](double& p){return p / sum; });
			}
		}
		assert(end == it);
	}

	/** Ensure that the initial guess for the initial probability distribution satifies our bounds and standard conditions for
	probability distributions */
	static void fix_init_guess_p0(std::vector<double>& x, const unsigned int dim, const unsigned int state_dim) {
		const auto begin = x.begin() + state_dim * dim;
		// Enforce lower and upper bounds
		for (auto it = begin; it != x.end(); ++it) {
			if (*it < LOWER_BOUND) {
				*it = LOWER_BOUND;
			} else if (*it > 1) {
				*it = 1;
			}
		}

		// Enforce that initial distribution sums to 1
		const double sum = std::accumulate(begin, x.end(), 0.0);
		if (std::abs(sum - 1.0) > 1E-12) {
			std::for_each(begin, x.end(), [sum](double& p) {return p / sum; });
		}
	}

	void CSM::set_stopping_conditions(nlopt::opt& opt, const StoppingConditions& stopping_conditions) {
		opt.set_stopval(stopping_conditions.stopval);
		opt.set_ftol_abs(stopping_conditions.ftol_abs);
		opt.set_ftol_rel(stopping_conditions.ftol_rel);
		opt.set_xtol_abs(stopping_conditions.xtol_abs);
		opt.set_xtol_rel(stopping_conditions.xtol_rel);
		opt.set_maxeval(stopping_conditions.maxeval);
		opt.set_maxtime(stopping_conditions.maxtime);
	}

	double CSM::estimate(nlopt::opt& opt, Eigen::MatrixXd& pi, Eigen::VectorXd& q0, nlopt::result& nlopt_result) {
		assert(static_cast<unsigned int>(pi.rows()) == _prms.dim);
		assert(static_cast<unsigned int>(pi.cols()) == _state_dim);
		assert(static_cast<unsigned int>(q0.size()) == _state_dim);		

		opt.set_min_objective(nlopt_csm_f, _f.get()); // set the function to minimize
		opt.set_lower_bounds(_lb); // set lower bounds for probabilities
		opt.set_upper_bounds(_ub); // set upper bounds for probabilities

		set_stopping_conditions(opt, _stopping_conditions);

		std::copy(pi.data(), pi.data() + pi.size(), _x.begin()); // matrix -> flat vector
		fix_init_guess_pi(_x, _prms.dim, _state_dim);
		std::copy(q0.data(), q0.data() + _state_dim, _x.begin() + _nbr_coeffs_pi);
		fix_init_guess_p0(_x, _prms.dim, _state_dim);
		double value;
		nlopt_result = run_nlopt("CSM", opt, _x, value);
		CSMUtils::normalize_distributions(_x, _state_dim, _prms.dim); // normalize the distributions (optimizer can leave them slightly non-normalised)
		{
			std::vector<double> tmp;
			value = _f->value(_x, tmp, ADD_NORMALISATION_TERM); // recalculate the error value
		}
		// Copy calibrated results back to input / output containers.
		std::copy(_x.begin(), _x.begin() + _nbr_coeffs_pi, pi.data());
		std::copy(_x.begin() + _nbr_coeffs_pi, _x.end(), q0.data());
		
		return value;
	}	

	void CSM::calc_errors(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, Eigen::MatrixXd& errors) const
	{
		if (!_with_padding) {
			calc_errors_nopad(pi, q0, errors);
		} else {
			// select errors for years for which we had input data
			Eigen::MatrixXd padded_errors;
			calc_errors_nopad(pi, q0, padded_errors);
			errors.resize(_prms.dim, _T);
			for (unsigned int t = 0; t < _T; ++t) {
				errors.col(t) = padded_errors.col(_input_to_padded[t]);
			}
		}
	}

	void CSM::calc_initial_guess_pi(Eigen::MatrixXd& pi, CSM::TransitionMatrixInitialisationMethod init_method) const {
		switch (init_method) {
		case TransitionMatrixInitialisationMethod::IDENTITY:
			calc_initial_guess_pi_identity(pi);
			return;
		case TransitionMatrixInitialisationMethod::MAX_ENTROPY:
			calc_initial_guess_pi_max_entropy(pi);
			return;
		case TransitionMatrixInitialisationMethod::FROM_TRAJECTORIES:
			calc_initial_guess_pi_trajectories(pi, true);
			return;
		case TransitionMatrixInitialisationMethod::FROM_TRAJECTORIES_COMPLETE_ONLY:
			calc_initial_guess_pi_trajectories(pi, false);
			return;
		case TransitionMatrixInitialisationMethod::HEURISTIC:
			calc_initial_guess_pi_heuristic(pi);
			return;
		default:
			throw std::invalid_argument("CSM: unknown transition matrix init method");
		}
	}

	void CSM::calc_initial_guess_q0(Eigen::VectorXd& q0, InitialStateDistributionInitialisationMethod init_method) const {
		switch (init_method) {
		case InitialStateDistributionInitialisationMethod::MAX_ENTROPY:
			calc_initial_guess_q0_max_entropy(q0);
			break;
		case InitialStateDistributionInitialisationMethod::FROM_DATA:
			calc_initial_guess_q0_from_data(q0);
			break;
		default:
			throw std::invalid_argument("CSM: unknown initial state distribution init method");
		}
	}

	void CSM::calc_initial_guess_pi_heuristic(Eigen::MatrixXd& pi) const {
		bool use_initial_guess_from_trajectories;
		if (_has_trajectories) {
			if (_has_cross_sectional) {
				use_initial_guess_from_trajectories = _orig_data.nbr_surveys.mean() < static_cast<double>(_orig_data.ltimes.size());
			} else {
				use_initial_guess_from_trajectories = true;
			}
		} else {
			use_initial_guess_from_trajectories = false;
		}

		if (use_initial_guess_from_trajectories) {
			assert(_f);
			calc_initial_guess_pi_trajectories(pi, true);
		} else {
			calc_initial_guess_pi_identity(pi);
		}
	}

	void CSM::calc_initial_guess_pi_trajectories(Eigen::MatrixXd& pi, bool use_incomplete_data) const {
		if (_has_trajectories) {
			pi = Markov::transition_matrix(_orig_data, _prms.dim, _prms.memory, use_incomplete_data);			
		} else {
			throw std::logic_error("CSM: No longitudinal data, cannot estimate transition matrix from them.");
		}
	}

	void CSM::calc_initial_guess_pi_identity(Eigen::MatrixXd& pi) const {
		if (_prms.dim == _state_dim) {
			// No memory.
			pi.setIdentity(_state_dim, _state_dim);
		} else {
			pi.setZero(_prms.dim, _state_dim);
			// The index of the latest state changes most frequently.
			for (unsigned int c = 0; c < _state_dim; ++c) {
				const unsigned int translated_idx = c % _prms.dim;
				pi(translated_idx, c) = 1.0;
			}
		}
	}

	void CSM::calc_initial_guess_pi_max_entropy(Eigen::MatrixXd& pi) const {
		pi.setConstant(_prms.dim, _state_dim, 1.0 / _prms.dim);		
	}	

	void CSM::calc_initial_guess_q0_from_data(Eigen::VectorXd& q0) const {
		if (static_cast<unsigned int>(q0.size()) != _state_dim) {
			q0.resize(_state_dim);
		}
		// Assume almost 100% correlation between time points.
		std::fill(q0.data(), q0.data() + _state_dim, LOWER_BOUND); // set all elements of initial state to zero before setting some of them to non-zero values later
		MultiIndex mi(_prms.memory + 1, _prms.dim);
		Eigen::VectorXd init_distr_guess(_prms.dim);
		if (_f->probs().size()) {
			// Use first provided distribution as initial guess for the initial state of the Markov process.
			std::copy(_f->probs().begin(), _f->probs().begin() + _prms.dim, init_distr_guess.data());
		} else {
			init_distr_guess = _data_reduced.probs.col(0);
		}
		while (mi.flat_index() < mi.flat_size()) {
			const size_t i = mi.indices().front();
			if (std::all_of(mi.indices().begin() + 1, mi.indices().end(), [i](const size_t i2) {return i2 == i; })) { // check if all indices are equal
				q0[mi.flat_index()] = init_distr_guess[i];
			}
			++mi; // increment multi-dimensional index
		}
	}

	void CSM::calc_initial_guess_q0_max_entropy(Eigen::VectorXd& q0) const {
		if (static_cast<unsigned int>(q0.size()) != _state_dim) {
			q0.resize(_state_dim);
		}
		std::fill(q0.data(), q0.data() + _state_dim, 1. / static_cast<double>(_state_dim));
	}



	// If you change error, change this as well
	void CSM::calc_errors_nopad(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, Eigen::MatrixXd& errors) const
	{
		errors.resize(_prms.dim, _padded_T); // make room for results

		// temp variables
		Eigen::VectorXd tmp1(_state_dim); // copy the init. state
		tmp1 = q0;
		Eigen::VectorXd tmp2(_state_dim); // temp vector for updated state

		// We treat errors matrix as storage
		CSMUtils::reduce(tmp1.data(), _state_dim, errors.data(), _prms.dim, 0); // reduce to prob. distribution for t = 0; Eigen doesn't like passing q0 to reduce, so we pass its copy from tmp1

		for (unsigned int t = 1; t < _padded_T; ++t) {
			// apply transition matrix
			tmp2 = pi * tmp1;
			tmp1 = tmp2;

			CSMUtils::reduce(tmp1.data(), _state_dim, errors.data() + t * _prms.dim, _prms.dim, 0);
		}
		// calculate actual errors
		const std::vector<double>& weights = _f->weights();
		const std::vector<double>& p = _f->probs();
		for (unsigned int t = 0; t < _padded_T; ++t) {
			const double w_t = weights[t];
			for (unsigned int k = 0; k < _prms.dim; ++k) {
				errors(k, t) = (p[t * _prms.dim + k] - errors(k, t)) * w_t; // K-L divergence in low error limit is absolute error
			}
		}
	}

	double CSM::value(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, Eigen::MatrixXd& grad_over_pi, Eigen::VectorXd& grad_over_q0) const {
		std::vector<double> x(_arg_dim, 0.);
		std::vector<double> grad(_arg_dim, 0.);
		CSMUtils::copy_probabilities(pi, _prms.dim, q0, x, _prms.memory);
		const double val = _f->value(x, grad, ADD_NORMALISATION_TERM);
		grad_over_pi.resize(_prms.dim, _state_dim);
		grad_over_q0.resize(_state_dim);
		std::copy(grad.begin(), grad.begin() + _nbr_coeffs_pi, grad_over_pi.data());
		std::copy(grad.begin() + _nbr_coeffs_pi, grad.begin() + (_nbr_coeffs_pi + _state_dim), grad_over_q0.data());
		return val;
	}

	double CSM::value(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0) const {
		std::vector<double> x(_arg_dim, 0.);
		CSMUtils::copy_probabilities(pi, _prms.dim, q0, x, _prms.memory);
		std::vector<double> empty;
		return _f->value(x, empty, ADD_NORMALISATION_TERM);
	}

	double CSM::value(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, Eigen::VectorXd& gradient, Eigen::MatrixXd& jacobian) const {
		std::vector<double> x(_arg_dim, 0.);
		std::vector<double> grad(_arg_dim, 0.);
		std::vector<double> jac(_arg_dim * _arg_dim, 0.);
		CSMUtils::copy_probabilities(pi, _prms.dim, q0, x, _prms.memory);
		const double val = _f->value(x, grad, jac, ADD_NORMALISATION_TERM);
		gradient.resize(_arg_dim);
		jacobian.resize(_arg_dim, _arg_dim);
		std::copy(grad.begin(), grad.end(), gradient.data());
		std::copy(jac.begin(), jac.end(), jacobian.data());
		return val;
	}

	Eigen::MatrixXd CSM::extrapolate_analytic_confidence_intervals(const Eigen::MatrixXd& pi, const Eigen::VectorXd& q0, Eigen::MatrixXd& extrap_probs, Eigen::MatrixXd& extrap_probs_lower, Eigen::MatrixXd& extrap_probs_upper, double confidence_level) const {
		/*if (_has_trajectories) {
			throw std::logic_error("CSM: analytical confidence interval calculation not supported for longitudinal data");
		}*/
		if (!_has_cross_sectional) {
			throw std::logic_error("CSM: cross-sectional data required for analytical confidence interval calculation");
		}
		std::vector<double> x(_arg_dim, 0.);
		CSMUtils::copy_probabilities(pi, _prms.dim, q0, x, _prms.memory);
		Eigen::MatrixXd cov;
		_f->extrapolate(x, confidence_level, cov, extrap_probs, extrap_probs_lower, extrap_probs_upper);
		return cov;
	}		

	CSM::Model::Model(const CSMParams& params, bool keep_pi, TransitionMatrixInitialisationMethod pi_init_method,
		InitialStateDistributionInitialisationMethod q0_init_method,
		StoppingConditions stopping_conditions, nlopt::algorithm algorithm)
	    : _prms(params), _state_dim(Markov::calc_state_dim(params.dim, params.memory))
	    , _keep_pi(keep_pi)
	    , _pi_init_method(pi_init_method)
		, _q0_init_method(q0_init_method)
		, _stopping_conditions(stopping_conditions)
		, _algorithm(algorithm)
	{}

	void CSM::Model::set_initial_guesses(const CSM& tme) {
		if ((!_keep_pi) || _pi.isZero()) {
			tme.calc_initial_guess_pi(_pi, _pi_init_method);
		} else {
			// reuse previous value of _pi
		}
		tme.calc_initial_guess_q0(_q0, _q0_init_method);
	}

	double CSM::Model::operator()(const ObservedDiscreteData& data, const std::vector<double>& extrap_times, Eigen::MatrixXd& extrap_probs) {
		if (!data.times.size()) {
			throw std::domain_error("CSM: No data");
		}
		//std::cout << "nbr_surveys: " << data.nbr_surveys << std::endl;
		CSMParams params_for_data(_prms);
		params_for_data.dim = static_cast<unsigned int>(ObservedDiscreteData::dim(data));		
		CSM tme(data, params_for_data);
		tme.set_stopping_conditions(_stopping_conditions);
		tme.set_algorithm(_algorithm);
		set_initial_guesses(tme);		
		const double error = tme.estimate(_pi, _q0);
		/*std::cout << "pi: " << _pi << "\n";
          std::cout << "_q0: " << _q0 << "\n";*/
		CSMUtils::extrapolate(_pi, _q0, static_cast<int>(data.times[0]), extrap_times, extrap_probs);
		return error;
	}

	static double calc_entropy(const ObservedDiscreteData& data) {
		if (data.nbr_surveys.size() > 0) {
			return Statistics::shannon(data.probs, data.nbr_surveys);
		} else {
			return 0;
		}
	}

	std::pair<double, double> CSM::Model::operator()(const ObservedDiscreteData& calibration_data, const ObservedDiscreteData& test_data) {
	    CSMParams params_for_calibr_data(_prms);
	    params_for_calibr_data.dim = static_cast<unsigned int>(ObservedDiscreteData::dim(calibration_data));
	    CSM calibration_model(calibration_data, params_for_calibr_data);
		calibration_model.set_stopping_conditions(_stopping_conditions);
		calibration_model.set_algorithm(_algorithm);
	    set_initial_guesses(calibration_model);
		const double calibr_error = calibration_model.estimate(_pi, _q0);
	    const double calibr_entropy = calc_entropy(calibration_data);
	    CSMParams params_for_test_data(_prms);
	    params_for_test_data.dim = static_cast<unsigned int>(ObservedDiscreteData::dim(test_data));
	    CSM test_model(test_data, params_for_test_data);
		const double test_error = test_model.value(_pi, _q0);
	    const double test_entropy = calc_entropy(test_data);
	    // Return MINUS log-likelihood
	    return std::make_pair(calibr_entropy + calibr_error, test_entropy + test_error);
	}	

	static std::array<std::string, 5> _TRANSITION_MATRIX_INIT_METHOD_NAMES = {
		"IDENTITY", "MAX_ENTROPY", "FROM_TRAJECTORIES", "FROM_TRAJECTORIES_COMPLETE_ONLY", "HEURISTIC"
	};

	std::ostream& operator<<(std::ostream& os, const CSM::TransitionMatrixInitialisationMethod method) {
		os << _TRANSITION_MATRIX_INIT_METHOD_NAMES[static_cast<int>(method)];
		return os;
	}

	CSM::TransitionMatrixInitialisationMethod CSM::transition_matrix_initialisation_method_from_string(const std::string& name) {
		for (size_t i = 0; i < _TRANSITION_MATRIX_INIT_METHOD_NAMES.size(); ++i) {
			if (name == _TRANSITION_MATRIX_INIT_METHOD_NAMES[i]) {
				return static_cast<TransitionMatrixInitialisationMethod>(i);
			}
		}
		throw std::invalid_argument((boost::format("CSM: invalid transition matrix initialisation method name: %s") % name).str());
	}

	static std::array<std::string, 2> _INITIAL_STATE_DISTR_INIT_METHOD_NAMES = {
		"MAX_ENTROPY", "FROM_DATA"
	};

	CSM::InitialStateDistributionInitialisationMethod CSM::initial_state_distribution_initialisation_method_from_string(const std::string& name) {
		for (size_t i = 0; i < _INITIAL_STATE_DISTR_INIT_METHOD_NAMES.size(); ++i) {
			if (name == _INITIAL_STATE_DISTR_INIT_METHOD_NAMES[i]) {
				return static_cast<InitialStateDistributionInitialisationMethod>(i);
			}
		}
		throw std::invalid_argument((boost::format("CSM: invalid initial state distribution initialisation method name: %s") % name).str());
	}
}

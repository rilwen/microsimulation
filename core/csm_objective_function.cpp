/*
  (C) Averisera Ltd 2014-2019
*/
#include "csm_objective_function.hpp"
#include "csm.hpp"
#include "csm_regulariser.hpp"
#include "csm_utils.hpp"
#include "csm_workspace.hpp"
#include "log.hpp"
#include "moore_penrose.hpp"
#include "normal_distribution.hpp"
#include "observed_discrete_data.hpp"
#include "preconditions.hpp"
#include "stl_utils.hpp"
#include <boost/format.hpp>
#include "utils.hpp"
#include <cassert>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <numeric>

namespace averisera {

	CSMObjectiveFunction::CSMObjectiveFunction(const ObservedDiscreteData& data, const CSMParams& params)
        : _p(data.probs.data(), data.probs.data() + data.probs.size()) // !!! Assumes column-major layout in Eigen !!!
        , _weights(data.nbr_surveys.data(), data.nbr_surveys.data() + data.nbr_surveys.size())
        , _sum_weights(data.nbr_surveys.sum())
        , _T(static_cast<unsigned int>(ObservedDiscreteData::T(data)))
        , _params(params.with_dim(data))
        , _state_dim(Markov::calc_state_dim(_params.dim, _params.memory))
        , _nbr_pi_coeffs(Markov::nbr_pi_coeffs(_params.dim, _params.memory))
        , _arg_dim(CSM::calc_arg_dim(_params.dim, _params.memory))
        , _n_sum_constrs(_params.dim + 1)
        , _trajs(data.ltrajs)
        , _traj_times(data.ltimes)
        , _min_time(ObservedDiscreteData::first_time(data))
        , _max_time(ObservedDiscreteData::last_time(data))
    {
		params.validate();
		//check_that(all_longitudinal_trajectories_continuous(_traj_times), "Longitudinal trajectories must be continuous");
        check_equals(_trajs.size(), _traj_times.size());
        check_equals(_trajs.nbr_elements(), _traj_times.nbr_elements());
        _wksp = std::make_shared<CSMWorkspace<0>>(_weights, _p, _T, _params.dim, _params.memory, data, _min_time);		

		// Call at the end of the constructor.
		if (params.regulariser) {
			params.regulariser->check_compatibility(*this);
		}
    }

    /* unused
       static bool all_longitudinal_trajectories_continuous(const Jagged2DArray<double>& ltimes) {
       bool continuous = true;
       for (auto rit = ltimes.row_begin(); rit != ltimes.row_end(); ++rit) {
       const auto row = *rit;
       const size_t len = row.size();
       for (size_t i = 1; i < len; ++i) {
       continuous &= (row[i] - row[i - 1]) == 1;
       }
       }
       return continuous;
       }*/

    // Calculate the (possibly reduced) state element index corresponding to the segment [i0, i1) of the trajectory
    template <class V> static size_t calc_state_idx(const unsigned int dim, const V& trajectory, const size_t i0, const size_t i1) {
        size_t idx = 0;
        for (size_t i = i0; i < i1; ++i) {
            idx = idx * dim + trajectory[i]; // index of the last time's value changes the fastest
        }
        return idx;
    }


    double CSMObjectiveFunction::value(const std::vector<double>& x, std::vector<double>& grad, bool add_normalization_term) const
    {
        assert(_wksp);
        return value<0>(*_wksp, x, grad, add_normalization_term);
    }

    double CSMObjectiveFunction::value(const std::vector<double>& x, std::vector<double>& grad, std::vector<double>& jacobian, bool add_normalization_term) const {
        assert(_wksp);
        std::vector<NestedADouble<1>::value_type> grad_ad(_arg_dim);
        CSMWorkspace<1> wksp1(*_wksp);
        const auto val = value<1>(wksp1, x, grad_ad, add_normalization_term);
        grad.resize(_arg_dim);
        std::transform(grad_ad.begin(), grad_ad.end(), grad.begin(), [](const NestedADouble<1>::value_type& v){return v.val(); });
        for (unsigned int i = 0; i < _arg_dim; ++i) {
            for (unsigned int j = 0; j < _arg_dim; ++j) {
                jacobian[i * _arg_dim + j] = grad_ad[i].dx(j);
            }
        }
        return val.val();
    }

    template <unsigned int L> typename CSMWorkspace<L>::value_t CSMObjectiveFunction::value(CSMWorkspace<L>& wksp, const std::vector<double>& x, std::vector<typename CSMWorkspace<L>::value_t>& grad, const bool add_normalization_term) const {
        typedef typename CSMWorkspace<L>::ad_scalar_t ad_scalar_t;

        // If the optimiser does not need the gradient, grad is an empty vector
        assert(grad.empty() || grad.size() == _arg_dim);

        const auto normalization_term = wksp.set_calibrated_parameters(x, !grad.empty());

        // calculate approximate probabilities
        extrapolate<ad_scalar_t>(wksp.pi_expanded, nullptr, _T, wksp.state_distr_approx, wksp.p_approx);

        // calculate sum of errors for the cross-sectional data
        typename CSMWorkspace<L>::ad_scalar_t total_error(0.);

        if (add_normalization_term) {
            total_error += normalization_term;
        }

		for (unsigned int t = 0; t < _T; ++t) {
            total_error += _weights[t] * unweighted_error(&_p[t * _params.dim], &wksp.p_approx[t * _params.dim], _params.dim);
        }

        if (_params.regulariser) {
			_params.regulariser->operator()(wksp);
			total_error += _params.regularisation_lambda * wksp.regularisation_term;
        }

        // subtract log-likelihood of longitudinal data
        auto traj_times_iter = _traj_times.row_begin();
        const auto traj_end = _trajs.row_end();
        size_t traj_idx = 0;
        const auto& nbr_specified_states = wksp.nbr_specified_states;
        const auto& expanded_data = wksp.expanded_data;
        auto& prev_state_distr = wksp.prev_state_distr;
        auto& next_state_distr = wksp.next_state_distr;
        auto& state_indices = wksp.state_indices;
        auto& state_multi_index = wksp.state_multi_index;
        for (auto traj_iter = _trajs.row_begin(); traj_iter != traj_end; ++traj_iter, ++traj_times_iter, ++traj_idx) {
            const auto traj = *traj_iter;
            const size_t traj_T = traj.size();
            const auto traj_times = *traj_times_iter;
            assert(traj_times.size() == traj_T);
            if (traj_T) {
                const double traj_min_time = traj_times[0];
                const unsigned int offset = static_cast<unsigned int>(traj_min_time - _min_time);
                if (_params.memory == 0) {
                    const auto p_init = wksp.p_approx[offset * _params.dim + traj[0]];
                    if (p_init == 0.) {
                        LOG_FATAL() << "CSMObjectiveFunction: Zero initial probability for trajectory " << traj_idx << ", traj[0] == " << traj[0];
                        throw std::runtime_error("CSMObjectiveFunction: Zero initial trajectory probability");
                    }
                    total_error -= log(p_init);
                    for (size_t t = 1; t < traj_T; ++t) {
                        const ObservedDiscreteData::lcidx_t k = traj[t];
                        assert(k < _params.dim);
                        const ObservedDiscreteData::lcidx_t l = traj[t - 1];
                        assert(l < _params.dim);
                        const unsigned int q = static_cast<unsigned int>(traj_times[t] - traj_times[t - 1]);
                        assert(q > 0);
                        const auto pi_q_kl = wksp.pi_power(q, k, l); // (pi^q)_{kl} is the probability of going from state l to state k in q steps
                        if (pi_q_kl == 0.) {
							LOG_FATAL() << "CSMObjectiveFunction: Zero transition probability for trajectory " << traj_idx << ", k == " << k << ", l == " << l << ", q == " << q;
                            throw std::runtime_error("CSMObjectiveFunction: Zero transition probability");
                        }
                        total_error -= log(pi_q_kl);
                    }
                } else {
                    const auto expanded_trajectory = expanded_data[traj_idx];

                    unsigned int st_idx1 = 0;
                    for (auto next_it = next_state_distr.begin(); next_it != next_state_distr.end(); ++next_it, ++st_idx1) {
                        if (st_idx1 % _params.dim == traj[0]) {
                            *next_it = wksp.state_distr_approx[offset * _state_dim + st_idx1];
                        } else {
                            *next_it = 0.0;
                        }
                    }

                    const unsigned int traj_max_time_idx = offset + static_cast<unsigned int>(traj_times[traj_T - 1] - traj_min_time) + 1;
                    assert(traj_max_time_idx <= _T);
                    unsigned int pi_power = 0;
                    const auto nbr_specified_states_row = nbr_specified_states[traj_idx];
                    for (unsigned int t = offset + 1; t < traj_max_time_idx; ++t) {
                        ++pi_power;
                        const ObservedDiscreteData::index_t nbr_specified = nbr_specified_states_row[t];
                        if (nbr_specified) {
                            prev_state_distr.swap(next_state_distr);
                            if (nbr_specified == _params.memory + 1) {
                                // Fully specified
                                // only 1 state is compatible
                                assert(pi_power == 1);
                                std::fill(next_state_distr.begin(), next_state_distr.end(), ad_scalar_t(0.0));
                                for (unsigned int q = 0; q <= _params.memory; ++q) {
                                    state_indices[q] = expanded_trajectory[t - q];
                                }
                                st_idx1 = static_cast<unsigned int>(MultiIndex::flatten(_params.dim, state_indices));
                                ad_scalar_t tmp(0.0);
                                unsigned int st_idx2 = 0;
                                for (auto prev_it = prev_state_distr.begin(); prev_it != prev_state_distr.end(); ++prev_it, ++st_idx2) {
                                    // If state_index2 is not compatible with previous data, then prev[state_index2.flat_index()] == 0.0
                                    if (*prev_it != 0.) {
                                        tmp += (*prev_it) * wksp.pi_expanded[st_idx2 * _state_dim + st_idx1];
                                    }
                                }
                                next_state_distr[st_idx1] = tmp;
                            } else {
                                // Partially specified
                                state_multi_index.reset();
                                for (auto next_it = next_state_distr.begin(); next_it != next_state_distr.end(); ++next_it, ++state_multi_index) {
                                    ad_scalar_t tmp(0.0);
                                    if (ObservedDiscreteData::state_index_compatible_with_data(expanded_trajectory, state_multi_index.indices(), _params.memory, t)) {
                                        unsigned int st_idx2 = 0;
                                        for (auto prev_it = prev_state_distr.begin(); prev_it != prev_state_distr.end(); ++prev_it, ++st_idx2) {
                                            // If state_index2 is not compatible with previous data, then prev[state_index2.flat_index()] == 0.0
                                            if (*prev_it != 0.) {
                                                tmp += (*prev_it) * wksp.pi_power(pi_power, static_cast<unsigned int>(state_multi_index.flat_index()), st_idx2);
                                            }
                                        }
                                    }
                                    *next_it = tmp;
                                }
                            }
                            pi_power = 0;
                        }
                    }
                    const auto trajectory_probability = std::accumulate(next_state_distr.begin(), next_state_distr.end(), ad_scalar_t(0.0));
                    if (trajectory_probability == 0.) {
						LOG_FATAL() << "CSMObjectiveFunction: Zero trajectory probability for trajectory " << traj_idx;
                        throw std::runtime_error("CSMObjectiveFunction: Zero trajectory probability");
                    }
                    total_error -= log(trajectory_probability);					
                }
            }
        }
        assert(traj_times_iter == _traj_times.row_end());

        // Copy gradient coefficients over pi if required.
        // There is no need to copy the gradient coefficients over initial state because we saved them in grad already (if required)
        if (!grad.empty()) {
            for (unsigned int i = 0; i < _arg_dim; ++i) {
                grad[i] = total_error.dx(i);
            }
        }

        // Return the error of the approximation
        return total_error.val();
    }

    template <class Scalar> void CSMObjectiveFunction::extrapolate(const std::vector<Scalar>& pi_expanded, const Scalar* q0, const unsigned int T, std::vector<Scalar>& state_distr_approx, std::vector<Scalar>& p_approx) const {
		assert(pi_expanded.size() == _state_dim * _state_dim);
		assert(state_distr_approx.size() == T * _state_dim);
		assert(p_approx.size() == T * _params.dim);
        if (q0) {
            std::copy(q0, q0 + _state_dim, &state_distr_approx[0]);
            CSMUtils::reduce(&state_distr_approx[0], _state_dim, &p_approx[0], _params.dim, 0);
        }
        for (unsigned int t = 1; t < T; ++t) {
			const auto* const prev_state_distr_approx = &state_distr_approx[(t - 1) * _state_dim];
            auto* const next_state_distr_approx = &state_distr_approx[t * _state_dim];			

            for (unsigned int k = 0; k < _state_dim; ++k) {
                Scalar tmp(0.0);
                for (unsigned int l = 0; l < _state_dim; ++l) {
                    // pi has column-major layout
                    tmp += pi_expanded[l * _state_dim + k] * prev_state_distr_approx[l];
                }
				next_state_distr_approx[k] = tmp;
            }
            CSMUtils::reduce(next_state_distr_approx, _state_dim, &p_approx[t * _params.dim], _params.dim, 0); // Calculate new fitted marginal distribution for time t
        }
    }

	void CSMObjectiveFunction::calc_param_covariance_matrix(const std::vector<double>& solution, const std::vector<double>& dpdx, Eigen::MatrixXd& cov) const {
		assert(solution.size() == _arg_dim);
		assert(dpdx.size() == _arg_dim);
		std::vector<CSMWorkspace<1>::value_t> grad_ad(_arg_dim);
        CSMWorkspace<1> wksp1(*_wksp);
        const auto val = value<1>(wksp1, solution, grad_ad, false);

		double total_gradient_norm = 0;
		cov.resize(_arg_dim, _arg_dim);
		for (unsigned int i = 0; i < _arg_dim; ++i) {
			total_gradient_norm += pow(grad_ad[i].val(), 2);
			for (unsigned int j = 0; j <= i; ++j) {				
				cov(i, j) = grad_ad[i].dx(j); // we're minimizing -log_likelihood and C^{-1}_{ij} = -\partial^2 L / \partial x_k \partial x_l = \partial^2 f / \partial x_k \partial x_l
				cov(j, i) = cov(i, j);
			}
		}
		total_gradient_norm = sqrt(total_gradient_norm);
		LOG_DEBUG() << "Total gradient norm for covariance matrix calculation: " << total_gradient_norm;

		// Convert the d^2L/dp_i dp_j to d^2L/dx_i dx_j, where x_i is the model parameter parameterising the probability p_i.
		for (unsigned int i = 0; i < _arg_dim; ++i) {
			const double dpdx_i = dpdx[i];
			cov(i, i) *= dpdx_i * dpdx_i;
			for (unsigned int j = 0; j < i; ++j) {
				const double tmp = dpdx_i * dpdx[j];
				cov(i, j) *= tmp;
				cov(j, i) *= tmp;
			}
		}

		// Invert matrix projected on the complement of its kernel (which we approximate by setting all eigenvalues lower than 1e-14 in absolute terms to zero).
        MoorePenrose::inverse(cov, 1E-14, cov);
    }

    void CSMObjectiveFunction::extrapolate(const std::vector<double>& solution, const double confidence_interval, Eigen::MatrixXd& cov, Eigen::MatrixXd& probs, Eigen::MatrixXd& lower, Eigen::MatrixXd& upper) const {
        const auto T = static_cast<unsigned int>(probs.cols());
        check_equals(static_cast<unsigned int>(probs.rows()), _params.dim, "Extrapolated probability distributions must have length equal to observed process dimension");
        check_equals(lower.rows(), probs.rows());
        check_equals(lower.cols(), probs.cols());
        check_equals(upper.rows(), probs.rows());
        check_equals(upper.cols(), probs.cols());
        _wksp->set_calibrated_parameters(solution, true);
        std::vector<CSMWorkspace<0>::ad_scalar_t> state_distr_approx(T * _state_dim);
		// probabilities
        std::vector<CSMWorkspace<0>::ad_scalar_t> p_approx(T * _params.dim);
		// logits l = log(p / (1 - p))
		std::vector<CSMWorkspace<0>::ad_scalar_t> logits(T * _params.dim);
        extrapolate(_wksp->pi_expanded, &_wksp->state_distr_approx[0], T, state_distr_approx, p_approx);
        // copy extrapolated probs
        std::transform(p_approx.begin(), p_approx.end(), probs.data(), [](const CSMWorkspace<0>::ad_scalar_t& p) { return p.val(); });
        // transform to logits
        std::transform(p_approx.begin(), p_approx.end(), logits.begin(), [](const CSMWorkspace<0>::ad_scalar_t& p) { return MathUtils::prob_to_logit(p); });
		std::vector<double> dpdx(_arg_dim, 1.);
		for (unsigned int i = 0; i < _arg_dim; ++i) {
			dpdx[i] = MathUtils::logit_to_prob_derivative(MathUtils::prob_to_logit(solution[i]));
		}
        calc_param_covariance_matrix(solution, dpdx, cov);
		const auto n_par_ind = static_cast<unsigned int>(cov.rows());
        Eigen::MatrixXd deltas(n_par_ind, T * _params.dim);
        for (unsigned int t = 0; t < T; ++t) {
            for (unsigned int k = 0; k < _params.dim; ++k) {
                const unsigned int idx = t * _params.dim + k;
				const auto& v = logits[idx];
                for (unsigned int i = 0; i < n_par_ind; ++i) {
                    deltas(i, idx) = v.dx(i) * dpdx[i];
                }
            }
        }
		Eigen::VectorXd sigmas;
		try {
			sigmas = Statistics::standard_deviations_delta(cov, deltas, 1e-8);
		} catch (std::runtime_error&) {
			throw std::runtime_error("Model parameter covariance matrix cannot be estimated, consider regularising the model");
		}
        const double percentile_lower = (1 - confidence_interval) / 2;
        const double sigma_scaling_factor = -NormalDistribution::normsinv(percentile_lower);
        for (unsigned int t = 0; t < T; ++t) {
            for (unsigned int k = 0; k < _params.dim; ++k) {
                const double delta = sigma_scaling_factor * sigmas[t * _params.dim + k];
                lower(k, t) = MathUtils::logit_to_prob(logits[t * _params.dim + k].val() - delta);
                upper(k, t) = MathUtils::logit_to_prob(logits[t * _params.dim + k].val() + delta);
            }
        }
    }
}

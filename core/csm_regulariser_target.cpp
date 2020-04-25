/*
(C) Averisera Ltd 2019
*/
#include "csm_objective_function.hpp"
#include "csm_regulariser_target.hpp"
#include "csm_workspace.hpp"
#include <stdexcept>

namespace averisera {

	CSMRegulariserTarget::CSMRegulariserTarget(const Eigen::MatrixXd& target_pi, const Eigen::VectorXd& target_q0, double pi_weight, double q0_weight) :
		target_pi_(target_pi),
		target_q0_(target_q0),
		pi_weight_(pi_weight),
		q0_weight_(q0_weight) {
		if (!target_pi_.size()) {
			throw std::invalid_argument("CSMRegulariserTarget: Empty target transition matrix");
		}
		if (!target_q0_.size()) {
			throw std::invalid_argument("CSMRegulariserTarget: Empty target initial state distribution");
		}
		if (target_pi_.cols() != target_q0_.size()) {
			throw std::invalid_argument("CSMRegulariserTarget: Markov state dimension mismatch");
		}
		if (target_pi_.cols() % target_q0_.rows() != 0) {
			throw std::invalid_argument("CSMRegulariserTarget: Markov state dimension not divisible by observed state dimension");
		}
	}

	void CSMRegulariserTarget::operator()(CSMWorkspaceNoData<0>& wksp) const {
		calc(wksp);
	}

	void CSMRegulariserTarget::operator()(CSMWorkspaceNoData<1>& wksp) const {
		calc(wksp);
	}

	template <unsigned int L> void CSMRegulariserTarget::calc(CSMWorkspaceNoData<L>& wksp) const {
		typename CSMWorkspaceNoData<L>::ad_scalar_t pi_sum(0.);
		auto ax_iter = wksp.ax.begin();
		const auto ax_pi_end = ax_iter + (wksp.arg_dim - wksp.state_dim);
		const double* target_ptr = target_pi_.data();
		while (ax_iter != ax_pi_end) {
			const auto delta = *ax_iter - *target_ptr;
			pi_sum += delta * delta;
			++ax_iter;
			++target_ptr;
		}
		typename CSMWorkspaceNoData<L>::ad_scalar_t q0_sum(0.);
		while (ax_iter != wksp.ax.end()) {
			const auto delta = *ax_iter - *target_ptr;
			q0_sum += delta * delta;
			++ax_iter;
			++target_ptr;
		}
		assert(ax_iter == wksp.ax.end());
		wksp.regularisation_term = pi_weight_ * pi_sum + q0_weight_ * q0_sum;
	}

	void CSMRegulariserTarget::check_compatibility(const CSMObjectiveFunction& objective_function) const {
		if (objective_function.state_dim() != static_cast<unsigned int>(target_q0_.size())) {
			throw std::invalid_argument("CSMRegulariserTarget: Target initial state distribution incompatible with the model");
		}
		if (objective_function.state_dim() != static_cast<unsigned int>(target_pi_.cols()) || objective_function.csm_params().dim != static_cast<unsigned int>(target_pi_.rows())) {
			throw std::invalid_argument("CSMRegulariserTarget: Target transition matrix incompatible with the model");
		}
	}
}
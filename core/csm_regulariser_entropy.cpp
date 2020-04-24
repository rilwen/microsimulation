/*
(C) Averisera Ltd 2019
*/
#include "csm_regulariser_entropy.hpp"
#include "csm_workspace.hpp"
#include <stdexcept>

namespace averisera {
	CSMRegulariserEntropy::CSMRegulariserEntropy(double pi_weight, double q0_weight)
		: pi_weight_(pi_weight), q0_weight_(q0_weight) {}

	void CSMRegulariserEntropy::operator()(CSMWorkspaceNoData<0>& wksp) const {
		calc(wksp);
	}

	void CSMRegulariserEntropy::operator()(CSMWorkspaceNoData<1>& wksp) const {
		calc(wksp);
	}

	template <unsigned int L> void CSMRegulariserEntropy::calc(CSMWorkspaceNoData<L>& wksp) const {
		typename CSMWorkspaceNoData<L>::ad_scalar_t pi_sum(0.);		
		auto ax_iter = wksp.ax.begin();
		const auto ax_pi_end = ax_iter + (wksp.arg_dim - wksp.state_dim);
		while (ax_iter != ax_pi_end) {
			pi_sum += (*ax_iter) * std::log((*ax_iter));
			++ax_iter;
		}
		typename CSMWorkspaceNoData<L>::ad_scalar_t q0_sum(0.);
		while (ax_iter != wksp.ax.end()) {
			q0_sum += (*ax_iter) * std::log((*ax_iter));
			++ax_iter;
		}
		assert(ax_iter == wksp.ax.end());
		wksp.regularisation_term = pi_weight_ * pi_sum + q0_weight_ * q0_sum;
	}
}
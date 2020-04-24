/*
(C) Averisera Ltd 2019
*/
#include "csm_regulariser_mixture.hpp"
#include "csm_workspace.hpp"
#include <cassert>
#include <stdexcept>

namespace averisera {
	CSMRegulariserMixture::CSMRegulariserMixture(const std::vector<std::shared_ptr<const CSMRegulariser>>& regularisers, const std::vector<double>& weights)
		: regularisers_(regularisers), weights_(weights) {
		if (regularisers.empty()) {
			throw std::invalid_argument("CSMRegulariserMixture: no components.");
		}
		if (regularisers.size() != weights.size()) {
			throw std::invalid_argument("CSMRegulariserMixture: different number of components and weights.");
		}
		for (const auto& ptr : regularisers) {
			if (!ptr) {
				throw std::invalid_argument("CSMRegulariserMixture: null component");
			}
		}
	}

	void CSMRegulariserMixture::operator()(CSMWorkspaceNoData<0>& wksp) const {
		calc(wksp);
	}

	void CSMRegulariserMixture::operator()(CSMWorkspaceNoData<1>& wksp) const {
		calc(wksp);
	}

	template <unsigned int L> void CSMRegulariserMixture::calc(CSMWorkspaceNoData<L>& wksp) const {
		typename CSMWorkspaceNoData<L>::ad_scalar_t sum(0.);
		auto reg_iter = regularisers_.begin();
		auto wgt_iter = weights_.begin();
		while (reg_iter != regularisers_.end()) {
			(**reg_iter)(wksp);
			sum += (*wgt_iter) * wksp.regularisation_term;
			++reg_iter;
			++wgt_iter;
		}
		assert(wgt_iter == weights_.end());
		wksp.regularisation_term = sum;
	}

	void CSMRegulariserMixture::check_compatibility(const CSMObjectiveFunction& objective_function) const {
		for (const auto& component_ptr : regularisers_) {
			component_ptr->check_compatibility(objective_function);
		}

	}
}
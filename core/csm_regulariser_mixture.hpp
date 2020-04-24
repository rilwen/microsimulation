/*
(C) Averisera Ltd 2019
*/
#pragma once
#include "csm_regulariser.hpp"
#include <memory>
#include <vector>

namespace averisera {
	/** Mixture of regularisers: Loss = sum_i c_i Loss_i.
	*/
	class CSMRegulariserMixture : public CSMRegulariser {
	public:
		/**
		@param regularisers Mixture components.
		@param weights Mixture weights.
		@throw std::invalid_argument If regularisers.empty(). If regularisers.size() != weights.size().
		*/
		CSMRegulariserMixture(const std::vector<std::shared_ptr<const CSMRegulariser>>& regularisers, const std::vector<double>& weights);

		void operator()(CSMWorkspaceNoData<0>& wksp) const override;

		void operator()(CSMWorkspaceNoData<1>& wksp) const override;

		void check_compatibility(const CSMObjectiveFunction& objective_function) const override;
	private:
		template <unsigned int L> void calc(CSMWorkspaceNoData<L>& wksp) const;

		std::vector<std::shared_ptr<const CSMRegulariser>> regularisers_;
		std::vector<double> weights_;
	};
}
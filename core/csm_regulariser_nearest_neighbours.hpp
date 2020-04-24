/*
(C) Averisera Ltd 2019
*/
#pragma once
#include "csm_regulariser.hpp"

namespace averisera {
	/** Regulariser which penalises transitions between states further away on an ordinal scale then given difference.
	
	P(X_{t + 1} = k | X_t = l, ...) is penalised with an L2 penalty for |k - l| > max_dist_not_penalised.
	*/
	class CSMRegulariserNearestNeighbours : public CSMRegulariser {
	public:
		/**
		@param max_dist_not_penalised Maximum value of |k - l| such that transition probabilities P(X_{t + 1} = k | X_t = l, ...) are not penalised.
		*/
		CSMRegulariserNearestNeighbours(unsigned int max_dist_not_penalised);

		void operator()(CSMWorkspaceNoData<0>& wksp) const override;

		void operator()(CSMWorkspaceNoData<1>& wksp) const override;

		void check_compatibility(const CSMObjectiveFunction& objective_function) const override {}
	private:
		template <unsigned int L> void calc(CSMWorkspaceNoData<L>& wksp) const;

		unsigned int max_dist_not_penalised_;
	};
}
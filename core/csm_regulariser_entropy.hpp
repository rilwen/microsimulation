/*
(C) Averisera Ltd 2019
*/
#pragma once
#include "csm_regulariser.hpp"

namespace averisera {
	/** Regulariser penalising high or low entropy.

	The cost function returned by regulariser is Sign * sum_i p_i log(p_i), where Sign = +/- 1.
	*/
	class CSMRegulariserEntropy : public CSMRegulariser {
	public:
		/**
		@param pi_weight Weight for transition matrix entropy.
		@param q0_weight Weight for initial state distribution entropy.
		*/
		CSMRegulariserEntropy(double pi_weight, double q0_weight);

		void operator()(CSMWorkspaceNoData<0>& wksp) const override;

		void operator()(CSMWorkspaceNoData<1>& wksp) const override;

		void check_compatibility(const CSMObjectiveFunction& /*objective_function*/) const override {}
	private:
		template <unsigned int L> void calc(CSMWorkspaceNoData<L>& wksp) const;

		double pi_weight_;
		double q0_weight_;
	};
}
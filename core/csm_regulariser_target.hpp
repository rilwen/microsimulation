/*
(C) Averisera Ltd 2019
*/
#pragma once
#include "csm_regulariser.hpp"
#include <Eigen/Core>

namespace averisera {

	/** Regulariser penalising deviation from target q0 and pi with L2 norm.
	TODO: Handle special cases of pi_weight == 0 or q0_weight == 0 more efficiently.
	*/
	class CSMRegulariserTarget : public CSMRegulariser {
	public:
		/**
		@param target_pi Target transition matrix.
		@param target_q0 Target initial state distribution.
		@param pi_weight Weight for transition matrix entropy.
		@param q0_weight Weight for initial state distribution entropy.
		@throw std::invalid_argument If the target_pi and target_q0 dimensions are incompatible.
		*/
		CSMRegulariserTarget(const Eigen::MatrixXd& target_pi, const Eigen::VectorXd& target_q0, double pi_weight, double q0_weight);

		void operator()(CSMWorkspaceNoData<0>& wksp) const override;

		void operator()(CSMWorkspaceNoData<1>& wksp) const override;

		void check_compatibility(const CSMObjectiveFunction& objective_function) const override;
	private:
		template <unsigned int L> void calc(CSMWorkspaceNoData<L>& wksp) const;

		Eigen::MatrixXd target_pi_;
		Eigen::VectorXd target_q0_;
		double pi_weight_;
		double q0_weight_;
	};
}
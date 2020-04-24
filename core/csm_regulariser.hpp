/*
	(C) Averisera Ltd 2019
*/
#pragma once

namespace averisera {

	class CSMObjectiveFunction;

	/// @tparam L nesting level of autodiff variables.
	template <unsigned int L> struct CSMWorkspaceNoData;

	/** @brief Abstract regulariser interface for CSM model.

	This interface is used by CSM to regularise the loss function. Implementations of this abstract
	base class have to take the CSMWorkspace<L> (for L == 0 or L == 1) object and calculate the value
	of the regularisation term, without the multiplicative constant. They should assume that the
	workspace contains the currently evaluated model parameters.
	*/
	class CSMRegulariser {
	public:
		virtual ~CSMRegulariser();

		/// Calculate the value of the regularisation term and store it in wksp.regularisation_term.
		/// @param wksp Workspace tracking 1st derivatives.
		virtual void operator()(CSMWorkspaceNoData<0>& wksp) const = 0;

		/// Calculate the value of the regularisation term and store it in wksp.regularisation_term.
		/// @param wksp Workspace tracking 1st and 2nd derivatives.
		virtual void operator()(CSMWorkspaceNoData<1>& wksp) const = 0;

		/// Check that the regulariser is compatible with the model.
		/// @throws std::invalid_argument If it's not.
		virtual void check_compatibility(const CSMObjectiveFunction& objective_function) const = 0;
	};
}
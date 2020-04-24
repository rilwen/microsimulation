#pragma once
#include "interpolator_impl_piecewise_polynomial.hpp"

namespace averisera {
	/** Specialisation of cubic polynomial */
	class InterpolatorImplPiecewiseCubic: public InterpolatorImplPiecewisePolynomial<2> {
	public:
		typedef InterpolatorImplPiecewisePolynomial<2>::DataNode DataNode;

		InterpolatorImplPiecewiseCubic(const std::vector<DataNode>& data);

		/** Is the interpolation monotonic in the i-th segment 
		@param rel_tol Return true even if the derivative changes sign within rel_tol * h distance from the node
		*/
		bool is_monotonic(size_t i, double rel_tol = 0) const;
	};
}

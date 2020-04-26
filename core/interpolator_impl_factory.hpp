// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_INTERP_INTERPOLATOR_IMPL_FACTORY_H
#define __AVERISERA_INTERP_INTERPOLATOR_IMPL_FACTORY_H

#include <memory>
#include <vector>

namespace averisera {
		class InterpolatorImpl;

		struct InterpolatorImplFactory
		{
			static std::shared_ptr<InterpolatorImpl> constant(double y);
			static std::shared_ptr<InterpolatorImpl> constant(double y, double lb);
			static std::shared_ptr<InterpolatorImpl> piecewiseConstant(const std::vector<double>& x, const std::vector<double>& y, bool leftInclusive);
			static std::shared_ptr<InterpolatorImpl> piecewiseLinear(const std::vector<double>& x, const std::vector<double>& y);
			static std::shared_ptr<InterpolatorImpl> piecewiseCubic(const std::vector<double>& x, const std::vector<double>& y, const std::vector<double>& dy);

			/** @see AkimaApproximation */
			static std::shared_ptr<InterpolatorImpl> akima(const std::vector<double>& x, const std::vector<double>& y);

			/** @see FritschButlandApproximation */
			static std::shared_ptr<InterpolatorImpl> fritschButland(const std::vector<double>& x, const std::vector<double>& y);

			/** @see KrugerApproximation */
			static std::shared_ptr<InterpolatorImpl> kruger(const std::vector<double>& x, const std::vector<double>& y);

			/** @see InterpolatorImplTanh */
			static std::shared_ptr<InterpolatorImpl> tanh(const std::vector<double>& x, const std::vector<double>& y, double lambda = 0.1);
		};
}

#endif // __AVERISERA_INTERP_INTERPOLATOR_IMPL_FACTORY_H

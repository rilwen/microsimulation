// (C) Averisera Ltd 2014-2020
#include "interpolator_impl_factory.hpp"
#include "interpolator_impl_piecewise_cubic.hpp"
#include "interpolator_impl_piecewise_constant.hpp"
#include "interpolator_impl_piecewise_polynomial.hpp"
#include "interpolator_impl_constant.hpp"
#include "interpolator_impl_tanh.hpp"
#include "akima_approximation.hpp"
#include "fritsch_butland_approximation.hpp"
#include "kruger_approximation.hpp"
#include <stdexcept>
#include <vector>

namespace averisera {

		std::shared_ptr<InterpolatorImpl> InterpolatorImplFactory::constant(double y)
		{
			return std::shared_ptr<InterpolatorImpl>(new InterpolatorImplConstant(y));
		}

        std::shared_ptr<InterpolatorImpl> InterpolatorImplFactory::constant(double y, double lb)
		{
			return std::shared_ptr<InterpolatorImpl>(new InterpolatorImplConstant(y, lb));
		}

		std::shared_ptr<InterpolatorImpl> InterpolatorImplFactory::piecewiseConstant(const std::vector<double>& x, const std::vector<double>& y, bool leftInclusive)
		{
			if (x.size() != y.size() + 1)
				throw std::domain_error("InterpolatorImplFactory::piecewiseConstant: x and y size mismatch");
			return std::shared_ptr<InterpolatorImpl>(new InterpolatorImplPiecewiseConstant(x, y, leftInclusive));
		}

		std::shared_ptr<InterpolatorImpl> InterpolatorImplFactory::piecewiseLinear(const std::vector<double>& x, const std::vector<double>& y)
		{
			const unsigned int size = static_cast<unsigned int>(x.size());
			if (y.size() != size)
				throw std::domain_error("InterpolatorImplFactory: x and y size mismatch");
			std::vector<typename InterpolatorImplPiecewisePolynomial<1>::DataNode> data(size);
			for (unsigned int i = 0; i < size; ++i)
			{
				data[i].x() = x[i];
				data[i].y()[0] = y[i];
			}
			return std::shared_ptr<InterpolatorImpl>(new InterpolatorImplPiecewisePolynomial<1>(data));
		}

		std::shared_ptr<InterpolatorImpl> InterpolatorImplFactory::piecewiseCubic(const std::vector<double>& x, const std::vector<double>& y, const std::vector<double>& dy)
		{
			const size_t size = x.size();
			if (y.size() != size)
				throw std::domain_error("InterpolatorImplFactory: x and y size mismatch");
			if (dy.size() != size)
				throw std::domain_error("InterpolatorImplFactory: x and dy size mismatch");
			std::vector<typename InterpolatorImplPiecewiseCubic::DataNode> data(size);
			for (unsigned int i = 0; i < size; ++i)
			{
				data[i].x() = x[i];
				data[i].y()[0] = y[i];
				data[i].y()[1] = dy[i];
			}
			return std::shared_ptr<InterpolatorImpl>(new InterpolatorImplPiecewiseCubic(data));
		}

		std::shared_ptr<InterpolatorImpl> InterpolatorImplFactory::akima(const std::vector<double>& x, const std::vector<double>& y)
		{
			const size_t size = x.size();
            if (y.size() != size) {
                throw std::domain_error("InterpolatorImplFactory: x and y size mismatch");
            }
			std::vector<double> dy(size);
			AkimaApproximation<double>::calculate(x, y, dy);
			return piecewiseCubic(x, y, dy);
		}

		std::shared_ptr<InterpolatorImpl> InterpolatorImplFactory::fritschButland(const std::vector<double>& x, const std::vector<double>& y)
		{
			const size_t size = x.size();
			if (y.size() != size) {
				throw std::domain_error("InterpolatorImplFactory: x and y size mismatch");
			}
			std::vector<double> dy(size);
			FritschButlandApproximation<double>::calculate(x, y, dy);
			return piecewiseCubic(x, y, dy);
		}

		std::shared_ptr<InterpolatorImpl> InterpolatorImplFactory::kruger(const std::vector<double>& x, const std::vector<double>& y)
		{
			const size_t size = x.size();
			if (y.size() != size) {
				throw std::domain_error("InterpolatorImplFactory: x and y size mismatch");
			}
			std::vector<double> dy(size);
			KrugerApproximation<double>::calculate(x, y, dy);
			return piecewiseCubic(x, y, dy);
		}

		std::shared_ptr<InterpolatorImpl> InterpolatorImplFactory::tanh(const std::vector<double>& x, const std::vector<double>& y, double lambda) {
			return std::shared_ptr<InterpolatorImpl>(new InterpolatorImplTanh(std::vector<double>(x), y, lambda));
		}
}

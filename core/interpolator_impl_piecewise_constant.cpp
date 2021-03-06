// (C) Averisera Ltd 2014-2020
#include "interpolator_impl_piecewise_constant.hpp"
#include <cassert>

namespace averisera {
		double InterpolatorImplPiecewiseConstant::evaluate_impl(double, size_t seg_idx) const
		{
			assert( seg_idx < m_y.size() );
			return m_y[seg_idx];
		}

		InterpolatorImplPiecewiseConstant::InterpolatorImplPiecewiseConstant(const std::vector<double>& x, const std::vector<double>& y, bool leftInclusive)
			: InterpolatorImplPiecewise(x, leftInclusive), m_y(y)
		{
			setup();
		}

		std::shared_ptr<InterpolatorImpl> InterpolatorImplPiecewiseConstant::clone() const
		{
			return std::shared_ptr<InterpolatorImpl>( new InterpolatorImplPiecewiseConstant(x(), m_y, left_inclusive()) );
		}

		InterpolatorImpl& InterpolatorImplPiecewiseConstant::operator+=(double x)
		{
			for (std::vector<double>::iterator it = m_y.begin(); it != m_y.end(); ++it)
				*it += x;
			return *this;
		}

		InterpolatorImpl& InterpolatorImplPiecewiseConstant::operator-=(double x)
		{
			for (std::vector<double>::iterator it = m_y.begin(); it != m_y.end(); ++it)
				*it -= x;
			return *this;
		}

		InterpolatorImpl& InterpolatorImplPiecewiseConstant::operator*=(double x)
		{
			for (std::vector<double>::iterator it = m_y.begin(); it != m_y.end(); ++it)
				*it *= x;
			return *this;
		}

		InterpolatorImpl& InterpolatorImplPiecewiseConstant::operator/=(double x)
		{
			for (std::vector<double>::iterator it = m_y.begin(); it != m_y.end(); ++it)
				*it /= x;
			return *this;
		}
}

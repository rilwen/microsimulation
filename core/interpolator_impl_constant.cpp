// (C) Averisera Ltd 2014-2020
#include "interpolator_impl_constant.hpp"
#include <limits>

namespace averisera {
		InterpolatorImplConstant::InterpolatorImplConstant(double value)
			: m_const(value), m_lower_bound(-std::numeric_limits<double>::infinity())
		{
		}

		InterpolatorImplConstant::InterpolatorImplConstant(double value, double lowerBound)
			: m_const(value), m_lower_bound(lowerBound)
		{
		}

		double InterpolatorImplConstant::evaluate(double) const
		{
			return m_const;
		}

		double InterpolatorImplConstant::lowerBound() const
		{
			return m_lower_bound;
		}

		double InterpolatorImplConstant::upperBound() const
		{
			return std::numeric_limits<double>::infinity();
		}

		std::shared_ptr<InterpolatorImpl> InterpolatorImplConstant::clone() const
		{
			return std::shared_ptr<InterpolatorImplConstant>(new InterpolatorImplConstant(m_const));
		}

		InterpolatorImpl& InterpolatorImplConstant::operator+=(double x)
		{
			m_const += x;
			return *this;
		}

		InterpolatorImpl& InterpolatorImplConstant::operator-=(double x)
		{
			m_const -= x;
			return *this;
		}

		InterpolatorImpl& InterpolatorImplConstant::operator*=(double x)
		{
			m_const *= x;
			return *this;
		}

		InterpolatorImpl& InterpolatorImplConstant::operator/=(double x)
		{
			m_const /= x;
			return *this;
		}
}

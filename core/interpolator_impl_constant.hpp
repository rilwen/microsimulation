// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_INTERPOLATOR_IMPL_CONSTANT_H
#define __AVERISERA_INTERPOLATOR_IMPL_CONSTANT_H

#include "interpolator_impl.hpp"

namespace averisera {
		class InterpolatorImplConstant: public InterpolatorImpl
		{
		public:
			InterpolatorImplConstant(double value);
			InterpolatorImplConstant(double value, double lowerBound);
			virtual double evaluate(double x) const;
			virtual double lowerBound() const;
			virtual double upperBound() const;
			virtual std::shared_ptr<InterpolatorImpl> clone() const;
			virtual InterpolatorImpl& operator+=(double x);
			virtual InterpolatorImpl& operator-=(double x);
			virtual InterpolatorImpl& operator*=(double x);
			virtual InterpolatorImpl& operator/=(double x);
		private:
			double m_const;
			double m_lower_bound;			
		};
}

#endif // __AVERISERA_INTERPOLATOR_IMPL_CONSTANT_H

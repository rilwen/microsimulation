// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_INTERPOLATOR_IMPL_H
#define __AVERISERA_INTERPOLATOR_IMPL_H

#include <memory>
#include <boost/utility.hpp>

namespace averisera {
	/** Interpolator implementation */
	class InterpolatorImpl: boost::noncopyable
	{
	public:
		virtual ~InterpolatorImpl();

		/** Evaluate interpolated function at x */
		virtual double evaluate(double x) const = 0;

		/** Lowest x value supported */
		virtual double lowerBound() const = 0;

		/** Highest x value supported */
		virtual double upperBound() const = 0;

		//! Perform a deep copy.
		virtual std::shared_ptr<InterpolatorImpl> clone() const = 0;

		//! Add x to interpolated function.
		virtual InterpolatorImpl& operator+=(double x) = 0;

		//! Subtract x from interpolated function.
		virtual InterpolatorImpl& operator-=(double x) = 0;

		//! Multiply the interpolated function by x.
		virtual InterpolatorImpl& operator*=(double x) = 0;

		//! Divide the interpolated function by x.
		virtual InterpolatorImpl& operator/=(double x) = 0;
	};
}

#endif // __AVERISERA_INTERPOLATOR_IMPL_H

// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_INTERPOLATOR_H
#define __AVERISERA_INTERPOLATOR_H

#include <cassert>
#include <memory>
#include <vector>
#include "pimpl.hpp"
#include "preconditions.hpp"
#include "segment_search.hpp"

namespace averisera {
	class InterpolatorImpl;

	// PIMPL idiom enables the use of Interpolator as a functor.
	// Relies on auto-generated copy operators/constructors.
	class Interpolator: public Pimpl<InterpolatorImpl>
	{
	public:
		Interpolator(std::shared_ptr<InterpolatorImpl> impl);

		/** Evaluate interpolated function at x */
		double operator()(double x) const;

		/** Lowest x value supported */
		double lowerBound() const;

		/** Highest x value supported */
		double upperBound() const;

		/** Add constant x to function */
		Interpolator operator+(double x) const;

		/** Subtract constant x from function */
		Interpolator operator-(double x) const;
		Interpolator operator*(double x) const;
		Interpolator operator/(double x) const;
		Interpolator& operator+=(double x);
		Interpolator& operator-=(double x);
		Interpolator& operator*=(double x);
		Interpolator& operator/=(double x);

		/** Given the vector of X values and corresponding Y values, interpolate the Y on new X values using (linear interpolation).
		@param x Sorted vector with unique values
		@param y Y vector
		@param new_x Sorted vector with unique values containing all values of x and some intermediate values.
		@throw std::domain_error If above conditions are not met.
		*/
		static std::vector<double> interpolate_y_linearly(const std::vector<double>& x, const std::vector<double>& y, const std::vector<double>& new_x);

		template <class VX, class VY, class X> static double interpolate_y_linearly(const VX& x, const VY& y, X new_x) {
			const size_t idx = SegmentSearch::binary_search_left_inclusive(x, new_x);
			assert(idx < x.size());
			const X x_idx = x[idx];
			if (x_idx == new_x) {
				return y[idx];
			} else {
				assert(new_x > x_idx);
				assert(idx + 1 < x.size());
				const X x_idx2 = x[idx + 1];
				check_that(x_idx2 > x_idx);
				return (y[idx] * (x_idx2 - new_x) + y[idx + 1] * (new_x - x_idx)) / (x_idx2 - x_idx);
			}
		}

		template <class VX, class VY, class X> static double interpolate_y_linearly_or_extrapolate(const VX& x, const VY& y, X new_x) {
			const size_t n = x.size();
			assert(n > 0);
			if (new_x < x[0]) {
				return y[0];
			}
			if (new_x > x[n - 1]) {
				return y[n - 1];
			}
			return interpolate_y_linearly(x, y, new_x);
		}
	};		
}

#endif // __AVERISERA_INTERPOLATOR_H

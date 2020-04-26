// (C) Averisera Ltd 2014-2020
#include "interpolator.hpp"
#include "interpolator_impl.hpp"
#include "preconditions.hpp"
#include "segment_search.hpp"

namespace averisera {

		Interpolator::Interpolator(std::shared_ptr<InterpolatorImpl> impl)
			: Pimpl<InterpolatorImpl>(impl)
		{
		}	

		std::vector<double> Interpolator::interpolate_y_linearly(const std::vector<double>& x, const std::vector<double>& y, const std::vector<double>& new_x) {
			check_that(y.size() > 1);
			check_equals(x.size(), y.size());
			check_equals(x.front(), new_x.front());
			check_equals(x.back(), new_x.back());
			const size_t n = new_x.size();
			check_that(x.size() <= n);
			std::vector<double> new_y(n);
			for (size_t i = 0; i < n; ++i) {
				new_y[i] = interpolate_y_linearly(x, y, new_x[i]);
			}
			return new_y;
		}

		double Interpolator::operator()(double x) const
		{
			return impl().evaluate(x);
		}

		double Interpolator::lowerBound() const
		{
			return impl().lowerBound();
		}

		double Interpolator::upperBound() const
		{
			return impl().upperBound();
		}

		Interpolator Interpolator::operator+(double x) const
		{
			std::shared_ptr<InterpolatorImpl> clone(impl().clone());
			*clone += x;
			return Interpolator(clone);
		}

		Interpolator Interpolator::operator-(double x) const
		{
			std::shared_ptr<InterpolatorImpl> clone(impl().clone());
			*clone -= x;
			return Interpolator(clone);
		}

		Interpolator Interpolator::operator*(double x) const
		{
			std::shared_ptr<InterpolatorImpl> clone(impl().clone());
			*clone *= x;
			return Interpolator(clone);
		}

		Interpolator Interpolator::operator/(double x) const
		{
			std::shared_ptr<InterpolatorImpl> clone(impl().clone());
			*clone /= x;
			return Interpolator(clone);
		}

		Interpolator& Interpolator::operator+=(double x)
		{
			impl() += x;
			return *this;
		}

		Interpolator& Interpolator::operator-=(double x)
		{
			impl() -= x;
			return *this;
		}

		Interpolator& Interpolator::operator*=(double x)
		{
			impl() *= x;
			return *this;
		}

		Interpolator& Interpolator::operator/=(double x)
		{
			impl() /= x;
			return *this;
		}
}

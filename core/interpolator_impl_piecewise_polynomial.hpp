	#ifndef __AVERISERA_INTERP_INTERPOLATOR_IMPL_PIECEWISE_POLYNOMIAL_H
#define __AVERISERA_INTERP_INTERPOLATOR_IMPL_PIECEWISE_POLYNOMIAL_H

#include "interpolator_impl_piecewise.hpp"
#include "polynomial_interpolation.hpp"
#include <array>
#include <vector>
#include <stdexcept>
#include <cassert>

namespace averisera {
		template <unsigned int N>
		class InterpolatorImplPiecewisePolynomial: public InterpolatorImplPiecewise
		{
		public:
			//! Stores the X, Y(X) and d^nY/dX^n values.
			class DataNode
			{
			public:
				DataNode()
					: DataNode(0.0) {}

				DataNode(double x)
					: DataNode(x, std::array<double, N>{}) {}

				//! @param[in] x Position
				//! @param[in] y Function value and optionally some derivatives
				DataNode(double x, const std::array<double,N>& y)
					: m_x(x), m_y(y)
				{}
				double x() const 
				{ 
					return m_x;
				}
				double& x()
				{ 
					return m_x;
				}
				const std::array<double,N>& y() const
				{
					return m_y;
				}
				std::array<double,N>& y()
				{
					return m_y;
				}
			private:
				double m_x;
				std::array<double,N> m_y;
			};
			static const unsigned int NUMBER_OF_MOMENTS = N;
			static const unsigned int POLYNOMIAL_ORDER = 2*N;
		public:
			/**
			@throw std::domain_error if data.size() < 2
			*/
			InterpolatorImplPiecewisePolynomial(const std::vector<DataNode>& data);

			InterpolatorImplPiecewisePolynomial()
				: InterpolatorImplPiecewisePolynomial(std::vector<DataNode>({ DataNode(0), DataNode(1) })) {}
			virtual std::shared_ptr<InterpolatorImpl> clone() const;
			virtual InterpolatorImpl& operator+=(double x);
			virtual InterpolatorImpl& operator-=(double x);
			virtual InterpolatorImpl& operator*=(double x);
			virtual InterpolatorImpl& operator/=(double x);
			typedef std::array<double, POLYNOMIAL_ORDER> coeffs_type;
			const coeffs_type& coeffs(size_t i) const {
				return m_a[i];
			}
		private:
			virtual double evaluate_impl(double x, size_t seg_idx) const;
			static std::vector<double> get_xs(const std::vector<DataNode>& data);
			InterpolatorImplPiecewisePolynomial(const std::vector<double>& x, const std::vector<std::array<double,POLYNOMIAL_ORDER>>& a);			
		private:
			//! Coefficients of an interpolating polynomial such that f(x) is approximated by sum_k a_k (x - eta(x))^k, where eta(x) is the left end of the segment to which x belongs.
			std::vector<coeffs_type> m_a;
		};

		template <unsigned int N> InterpolatorImplPiecewisePolynomial<N>::InterpolatorImplPiecewisePolynomial(const std::vector<typename InterpolatorImplPiecewisePolynomial<N>::DataNode>& data)
			: InterpolatorImplPiecewise(get_xs(data),true), m_a(data.size()-1) // get_xs will throw if data.size() < 2
		{
			for (size_t i = 0; i < nbr_segments(); ++i)
			{
				PolynomialInterpolation::interpolate(data[i+1].x() - data[i].x(), data[i].y(), data[i+1].y(), m_a[i]);
			}
		}

		template <unsigned int N> double InterpolatorImplPiecewisePolynomial<N>::evaluate_impl(double x, size_t seg_idx) const
		{
			const double dx = x - this->x()[seg_idx];
			assert( dx >= 0.0 );
			return PolynomialInterpolation::evaluate_poly<POLYNOMIAL_ORDER>(dx, m_a[seg_idx]);
		}

		template <unsigned int N> std::vector<double> InterpolatorImplPiecewisePolynomial<N>::get_xs(const std::vector<typename InterpolatorImplPiecewisePolynomial<N>::DataNode>& data)
		{
			if (data.size() < 2) {
				throw std::domain_error("InterpolatorImplPiecewisePolynomial: data size less than 2");
			}
			std::vector<double> xs;
			xs.reserve(data.size());
			for (typename std::vector<InterpolatorImplPiecewisePolynomial<N>::DataNode>::const_iterator it = data.begin(); it != data.end(); ++it)
				xs.push_back(it->x());
			return xs;
		}

		template <unsigned int N> InterpolatorImplPiecewisePolynomial<N>::InterpolatorImplPiecewisePolynomial(const std::vector<double>& x, const std::vector<std::array<double,POLYNOMIAL_ORDER>>& a)
			: InterpolatorImplPiecewise(x, true), m_a(a)
		{
		}

		template <unsigned int N> std::shared_ptr<InterpolatorImpl> InterpolatorImplPiecewisePolynomial<N>::clone() const
		{
			return std::shared_ptr<InterpolatorImpl>( new InterpolatorImplPiecewisePolynomial<N>(x(), m_a) );
		}

		template <unsigned int N> InterpolatorImpl& InterpolatorImplPiecewisePolynomial<N>::operator+=(double x)
		{
			for (typename std::vector<std::array<double,POLYNOMIAL_ORDER>>::iterator it = m_a.begin(); it != m_a.end(); ++it)
				(*it)[0] += x;
			return *this;
		}

		template <unsigned int N> InterpolatorImpl& InterpolatorImplPiecewisePolynomial<N>::operator-=(double x)
		{
			for (typename std::vector<std::array<double,POLYNOMIAL_ORDER>>::iterator it = m_a.begin(); it != m_a.end(); ++it)
				(*it)[0] -= x;
			return *this;
		}

		template <unsigned int N> InterpolatorImpl& InterpolatorImplPiecewisePolynomial<N>::operator*=(double x)
		{
			for (typename std::vector<std::array<double,POLYNOMIAL_ORDER>>::iterator it = m_a.begin(); it != m_a.end(); ++it)
			{
				for (unsigned int n = 0; n < POLYNOMIAL_ORDER; ++n)
					(*it)[n] *= x;				
			}
			return *this;
		}

		template <unsigned int N> InterpolatorImpl& InterpolatorImplPiecewisePolynomial<N>::operator/=(double x)
		{
			for (typename std::vector<std::array<double,POLYNOMIAL_ORDER>>::iterator it = m_a.begin(); it != m_a.end(); ++it)
			{
				for (unsigned int n = 0; n < POLYNOMIAL_ORDER; ++n)
					(*it)[n] /= x;				
			}
			return *this;
		}
}

#endif // __AVERISERA_INTERP_INTERPOLATOR_IMPL_PIECEWISE_POLYNOMIAL_H

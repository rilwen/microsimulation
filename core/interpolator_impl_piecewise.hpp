// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_INTERP_INTERPOLATOR_IMPL_PIECEWISE_H
#define __AVERISERA_INTERP_INTERPOLATOR_IMPL_PIECEWISE_H

#include <algorithm>
#include <cassert>
#include <vector>
#include "interpolator_impl.hpp"

namespace averisera {

		// Type of interpolator which is defined piecewise.
		class InterpolatorImplPiecewise: public InterpolatorImpl
		{
		public:
			double evaluate(double x) const override;
			double lowerBound() const override;
			double upperBound() const override;
		private:
			// x - initial argument
			// seg_idx - index of the LEFT segment boundary in m_x
			virtual double evaluate_impl(double x, size_t seg_idx) const = 0;
			void setup();
		protected:
			// Constructors are protected because this class needs to be extended in order to be used.

			// if LeftInclusive == true, find such i that x[i] <= z < x[i+1]
			// if LeftInclusive == false, find such i that x[i] < z <= x[i+1]
			// evaluate(z) is always well-defined for z == x[0] or z == x[x.size()-1]
			template <class V>
			InterpolatorImplPiecewise(const V& x, bool leftInclusive);
			template <class ItBegin, class ItEnd>
			InterpolatorImplPiecewise(ItBegin xBegin, ItEnd xEnd, bool leftInclusive);
			InterpolatorImplPiecewise(const std::vector<double>& x, bool leftInclusive);
			InterpolatorImplPiecewise(std::vector<double>&& x, bool leftInclusive);

			/** Default constructor on [0, 1) range */
			InterpolatorImplPiecewise()
				: InterpolatorImplPiecewise(std::vector<double>({ 0, 1 }), true) {}
			
			const std::vector<double>& x() const {
				return m_x;
			}

			bool left_inclusive() const {
				return m_left_inclusive;
			}

			size_t nbr_segments() const {
				return m_nbr_segments;
			}

			size_t find_segment(double x) const;
		private:
			std::vector<double> m_x;
			const bool m_left_inclusive;
			size_t m_nbr_segments;
		};

		template <class V>
		InterpolatorImplPiecewise::InterpolatorImplPiecewise(const V& x, bool leftInclusive)
			: m_x(x.size()), m_left_inclusive(leftInclusive)
		{
			size_t i = 0;
			for (std::vector<double>::iterator it = m_x.begin(); it != m_x.end(); ++it)
			{
				*it = x[i];
				++i;
			}
			setup();
		}

		template <class ItBegin, class ItEnd>
		InterpolatorImplPiecewise::InterpolatorImplPiecewise(ItBegin xBegin, ItEnd xEnd, bool leftInclusive)
			: m_x(0u), m_left_inclusive(leftInclusive)
		{
			std::copy(xBegin, xEnd, std::back_inserter(m_x));
			setup();
		}

		inline void InterpolatorImplPiecewise::setup()
		{
			if (m_x.size() < 2) {
				throw std::domain_error("InterpolatorImplPiecewise: m_x size less than 2");
			}
			m_nbr_segments = static_cast<size_t>(m_x.size() - 1);
		}
}

#endif // __AVERISERA_INTERP_INTERPOLATOR_IMPL_PIECEWISE_H

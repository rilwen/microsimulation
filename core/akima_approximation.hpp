/*
(C) Averisera Ltd 2014-2020
*/
#ifndef __AVERISERA_INTERP_AKIMA_APPROXIMATION_H
#define __AVERISERA_INTERP_AKIMA_APPROXIMATION_H

#include <cmath>
#include <vector>
#include <stdexcept>
#include "index_shifter.hpp"
#include <cassert>

namespace averisera {
		/** Approximates dy/dx given x and y(x) using the Akima algorithm
		@tparam ValueType Function value type
		*/
		template <class ValueType>
		struct AkimaApproximation
		{
			//! @tparam V1 Vector class with operator[] and .size()
			//! @tparam V2 Vector class with operator[] and .size()
			//! @tparam V3 Vector class with operator[] and .size()
			template <class V1, class V2, class V3>
			static void calculate(const V1& x, const V2& y, V3& dy);
		};

		template <class ValueType>
		template <class V1, class V2, class V3>
        void AkimaApproximation<ValueType>::calculate(const V1& x, const V2& y, V3& dy)
        {
            const int size = static_cast<int>(x.size());
            if (static_cast<int>(y.size()) != size)
                throw std::domain_error("AkimaApproximation: x and y size mismatch");
            if (static_cast<int>(dy.size()) != size)
                throw std::domain_error("AkimaApproximation: x and dy size mismatch");
            if (size < 2)
                throw std::domain_error("AkimaApproximation: needs at least 1 segment");
            const int nbr_segs = static_cast<int>(size - 1);

            ShiftedVector<std::vector<ValueType>, ValueType> d(std::vector<ValueType>(size + 3), -2);
            for (int i = 0; i < nbr_segs; ++i) {
                d[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]);
            }
			d[-1] = 2*d[0]-d[1];
			d[-2] = 2*d[-1]-d[0];
			d[nbr_segs] = 2*d[nbr_segs-1]-d[nbr_segs-2];
			d[nbr_segs+1] = 2*d[nbr_segs] - d[nbr_segs-2];

			// approximate the derivatives of y
            const int size_i = static_cast<int>(size);
			for (int i = 0; i < size_i; ++i)
			{
				const ValueType w_l = std::abs(d[i+1] - d[i]);
				const ValueType w_r = std::abs(d[i-1] - d[i-2]);
				if (w_l != 0 || w_r != 0) {
					dy[i] = (w_l*d[i-1] + w_r*d[i])/(w_l + w_r);
				} else {
					dy[i] = 0.5*(d[i - 1] + d[i]);
				}
			}
		}
}

#endif // __AVERISERA_INTERP_AKIMA_APPROXIMATION_H

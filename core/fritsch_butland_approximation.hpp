// (C) Averisera Ltd 2014-2020
#pragma once
#include "preconditions.hpp"

namespace averisera {
	/** Monotonicity-preserving C1 approximation */
	template <class ValueType> struct FritschButlandApproximation {
		/** @tparam V1 Vector class with operator[] and .size()
		@tparam V2 Vector class with operator[] and .size()
		@tparam V3 Vector class with operator[] and .size()
		*/
		template <class V1, class V2, class V3> static void calculate(const V1& x, const V2& y, V3& dy) {
			const size_t n = x.size();
			check_that(n > 1);
			check_equals(y.size(), n);
			check_equals(dy.size(), n);
			if (n > 2) {
				const auto h0 = x[1] - x[0];
				const auto s0 = (y[1] - y[0]) / h0;
 				const auto h1 = x[2] - x[1];
				const auto s1 = (y[2] - y[1]) / h1;
				const auto g0 = ((2 * h0 + h1) * s0 - h0 * s1) / (h0 + h1);

				if (g0 * s0 < 0) {
					dy[0] = 0;
				} else if (s0 * s1 <= 0 && std::abs(g0) > 3 * std::abs(s0)) {
					dy[0] = 3 * s0;
				} else {
					dy[0] = g0;
				}

				const auto hm0 = x[n - 1] - x[n - 2];
				const auto sm0 = (y[n - 1] - y[n - 2]) / hm0;
				const auto hm1 = x[n - 2] - x[n - 3];
				const auto sm1 = (y[n - 2] - y[n - 3]) / hm1;
				const auto gm0 = ((2 * hm0 + hm1) * sm0 - hm0 * sm1) / (hm0 + hm1);

				if (gm0 * sm0 < 0) {
					dy[n - 1] = 0;
				} else if (sm0 * sm1 <= 0 && std::abs(gm0) > 3 * std::abs(sm0)) {
					dy[n - 1] = 3 * sm0;
				} else {
					dy[n - 1] = gm0;
				}

				for (size_t i = n - 2; i > 0; --i) {
					const auto hr = x[i + 1] - x[i];
					const auto hl = x[i] - x[i - 1];
					const auto sr = (y[i + 1] - y[i]) / hr;
					const auto sl = (y[i] - y[i - 1]) / hl;
					if (sl * sr <= 0) {
						dy[i] = 0;
					} else {
						const auto w = 3 * (hl + hr);
						const auto inv_dy = (2 * hr + hl) / w / sl + (2 * hl + hr) / w / sr;
						dy[i] = 1.0 / inv_dy;
					}
				}
			} else {
				const auto s = (y[1] - y[0]) / (x[1] - x[0]);
				dy[0] = s;
				dy[1] = s;
			}
		}
	};
}

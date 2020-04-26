// (C) Averisera Ltd 2014-2020
#include "interpolator_impl_piecewise.hpp"
#include "segment_search.hpp"
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <cassert>


namespace averisera {
		InterpolatorImplPiecewise::InterpolatorImplPiecewise(const std::vector<double>& x, bool leftInclusive)
			: m_x(x), m_left_inclusive(leftInclusive)
		{
			setup();
		}

		InterpolatorImplPiecewise::InterpolatorImplPiecewise(std::vector<double>&& x, bool leftInclusive)
			: m_x(std::move(x)), m_left_inclusive(leftInclusive) {
			try {
				setup();
			} catch (std::exception& e) {
				// cleanup
				x = std::move(m_x);
				throw e;
			}
		}

		double InterpolatorImplPiecewise::evaluate(double x) const
		{
			if (x < m_x.front() || x > m_x.back()) {
				throw std::out_of_range("InterpolatorImplPiecewise: X outside range");
			}

			return evaluate_impl(x, find_segment(x));
		}

		size_t InterpolatorImplPiecewise::find_segment(double x) const {
			size_t segment_idx;
			if (m_left_inclusive)
			{
				segment_idx = std::min<size_t>(SegmentSearch::binary_search_left_inclusive(m_x, x), m_nbr_segments - 1);
				assert(segment_idx < SegmentSearch::NOT_FOUND);
			} else
			{
				segment_idx = SegmentSearch::binary_search_right_inclusive(m_x, x);
				if (segment_idx == SegmentSearch::NOT_FOUND) {
					segment_idx = 0;
				}
			}
			return segment_idx;
		}

		double InterpolatorImplPiecewise::lowerBound() const
		{
			return m_x.front();
		}

		double InterpolatorImplPiecewise::upperBound() const
		{
			return m_x.back();
		}				
}

#ifndef __AVERISERA_SEGMENT_SEARCH_H
#define __AVERISERA_SEGMENT_SEARCH_H

#include <limits>
#include <stdexcept>

namespace averisera {
	struct SegmentSearch
	{		
        static const size_t NOT_FOUND = std::numeric_limits<size_t>::max();

		template <class T, class V> struct default_getter {
			const T& operator()(const V& v, size_t i) {
				return v[i];
			}
		};
            
		//! Find such i that data[i] <= x < data[i+1], using binary search
		//! @param data Ordered vector of values
		//! @param size Size of data, non-zero
		//! @tparam V Vector type with operator[]
		//! @return Index i < size if a suitable segment is found OR NOT_FOUND if x < data[0]
		template <class V, class T, class G = default_getter<T, V>>
		static size_t binary_search_left_inclusive(const V& data, size_t size, const T& x, G g = default_getter<T, V>());

		//! Find such i that data[i] <= x < data[i+1], using binary search
		//! @param data Ordered vector of values
		//! @tparam V Vector type with operator[]
		//! @return Index i if a suitable segment is found OR NOT_FOUND if x < data[0]
		template <class V, class T>
		static size_t binary_search_left_inclusive(const V& data, const T& x)
		{
			return binary_search_left_inclusive<V, T>(data, static_cast<size_t>(data.size()), x);
		}

		//! Find such i that data[i] < x <= data[i+1], using binary search
		//! @param data Ordered vector of values
		//! @param size Size of data, non-zero.
		//! @tparam V Vector type with operator[] and .size() method
		//! @return Index i if a suitable segment is found OR NOT_FOUND if x <= data[0]. If x >= data.back(), return size.
		template <class V, class T, class G = default_getter<T, V>>
		static size_t binary_search_right_inclusive(const V& data, size_t size, const T& x, G g = default_getter<T, V>());

		//! Find such i that data[i] < x <= data[i+1], using binary search
		//! @param data Ordered vector of values
		//! @tparam V Vector type with operator[] and .size() method
		//! @return Index i if a suitable segment is found OR NOT_FOUND if x <= data[0] If x >= data.back(), return size.
		template <class V, class T>
		static size_t binary_search_right_inclusive(const V& data, const T& x)
		{
			return binary_search_right_inclusive<V, T>(data, static_cast<size_t>(data.size()), x);
		}

		/** Return largest i such that indices[i] <= dest, or 0. Used to pad missing data forward and (before the first given datapoint) backward.
		*/
		template <class V, class T, class G = default_getter<T, V>>
		static size_t find_index_for_padding_forward_and_backward(const V& indices, size_t size, const T& dest, G g = default_getter<T, V>()) {
			const size_t i = binary_search_left_inclusive<V, T, G>(indices, size, dest, g);
			return i != NOT_FOUND ? i : 0;
		}
	};

	template <class V, class T, class G> size_t SegmentSearch::binary_search_left_inclusive(const V& data, size_t size, const T& x, G g)
	{
		if (size == 0)
			throw std::domain_error("SegmentSearch: Zero-sized array");
		if (x < g(data, 0u))
			return NOT_FOUND;

		// there is a segment which contains x

		// logically, we assume that data[size] == +Infinity
		size_t l = 0; // highest i s.t. data[i] <= x
		size_t r = size; // lowest i s.t. data[i] > x
		size_t d = size;
		while (d > 1u)
		{
			const size_t m = l + d/2;
			if (x < g(data, m))
				r = m;
			else
				l = m;
			d = r - l;
		}
		return l;
	}

	template <class V, class T, class G> size_t SegmentSearch::binary_search_right_inclusive(const V& data, size_t size, const T& x, G g)
	{
		if (size == 0)
			throw std::domain_error("SegmentSearch: Zero-sized array");
		if (x <= g(data, 0u))
			return NOT_FOUND;

		// there is a segment which contains x

		// logically, we assume that data[size] == +Infinity
		size_t l = 0; // highest i s.t. data[i] < x
		size_t r = size; // lowest i s.t. data[i] >= x
		size_t d = size;
		while (d > 1)
		{
			const size_t m = l + d/2;
			if (x <= g(data, m))
				r = m;
			else
				l = m;
			d = r - l;
		}
		return l;
	}
}

#endif // __AVERISERA_SEGMENT_SEARCH_H

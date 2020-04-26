// (C) Averisera Ltd 2014-2020
#pragma once
#include "segment_search.hpp"
#include <cassert>
#include <type_traits>
#include <Eigen/Core>

namespace averisera {
	/** Functions related to padding missing values */
	struct Padding {
		/** Extrapolate forward all values and the first one also backward. */
		template <class ValueVector, class Index, class IndexVector> static auto get_padded(const IndexVector& iv, const ValueVector& vv, size_t size, Index idx) -> decltype(vv[0]) {
			return vv[SegmentSearch::find_index_for_padding_forward_and_backward(iv, size, idx)];
		}

		/** Extrapolate forward all values and the first one also backward. */
		template <class ValueVector, class Index, class IndexVector> static auto get_padded(const IndexVector& iv, const ValueVector& vv, Index idx) -> decltype(vv[0]) {
			return get_padded(iv, vv, vv.size(), idx);
		}

		/** Fill dest vector with values corresponding to indices [start_dest_idx, end_dest_idx) using padding */
		template <class ValueVector1, class ValueVector2, class Index, class IndexVector> static void pad(const IndexVector& known_indices, const ValueVector1& known_values, Index start_dest_idx, Index end_dest_idx, ValueVector2& dest) {			
			size_t i = 0;
			const size_t len = known_indices.size();
			assert(len == known_values.size());
			for (Index idx = start_dest_idx; idx < end_dest_idx; ++idx, ++i) {
				dest[i] = get_padded(known_indices, known_values, len, idx);
			}
		}

		/** Replace selected values by padding forward or backward (only at front). 
		@param predicate Returns true for selected values
		@return true if padding worked, false if all values were unknown
		*/
		template <class Vector, class Pred> static bool pad_selected(Vector& values, const Pred predicate) {
			const size_t len = values.size();
			size_t i = 0;
			while (i < len && predicate(values[i])) {
				++i;
			}
			if (i == len) {
				return false;
			}			
			auto v = values[i];
			for (size_t j = 0; j < i; ++j) {
				values[j] = v;
			}
			while (i < len) {
				v = values[i];
				++i;
				while (i < len && predicate(values[i])) {
					values[i] = v;
					++i;
				}
			}
			return true;
		}

		/** Replace unknown value by padding forward or backward (only at front).
		@return true if padding worked, false if all values were unknown
		*/
		template <class Vector, class Value> static bool pad_unknown(Vector& values, const Value unknown) {
			return pad_selected(values, [unknown](const Value& v) { return unknown == v; });
		}

		/** Replace unknown value by padding forward or backward (only at front).
		@return true if padding worked, false if all values were unknown
		*/
		template <class Vector> static bool pad_nan(Vector& values) {
			typedef typename std::remove_reference<decltype(values[0])>::type value_type;
			return pad_selected(values, static_cast<bool(*)(value_type)>(std::isnan));
		}

		/** Pad NaNs in each row 
		@return True if all rows padded OK
		*/
		static bool pad_nan_rows(Eigen::MatrixXd& m);

		/** Pad NaNs in each column 
		@return True if all cols padded OK
		*/
		static bool pad_nan_cols(Eigen::MatrixXd& m);
	};
}

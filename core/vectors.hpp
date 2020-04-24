/*
(C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_VECTORS_H
#define __AVERISERA_VECTORS_H

#include <vector>
#include <algorithm>
#include <cassert>
#include "preconditions.hpp"

namespace averisera {

	namespace Vectors {
		// Ensure there is an element with index i
		template <class V> void ensure_elem(size_t i, V& vec) {
			if (i >= vec.size()) {
				vec.resize(i + 1);
			}
		}

		// Ensure there is a range of elements with indices in the range [i0, i1)
		// i1 >= i0
		template <class V> void ensure_range(size_t i0, size_t i1, V& vec) {
			check_that(i1 >= i0);
			ensure_elem(i1 - 1, vec);
		}
	}

		
}

#endif
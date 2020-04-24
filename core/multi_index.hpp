/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_MULTI_INDEX_H
#define __AVERISERA_MULTI_INDEX_H

#include <cstdlib>
#include <vector>

namespace averisera {
	// Class representing a multidimensional index with constant dimension. E.g. a 2D MultIndex represents a pair
	// (i1, i2) in which 0 <= i1 < N and 0 <= i2 < N, with i1 changing faster.
	// Iteration example:
	// MultiIndex mi(2, 10);
	// while (mi.flat_index() < mi.flat_size()) {
	//		// do something
	//		++mi;
	// }
	class MultiIndex {
	public:
		// Construct MultiIndex with dimension dim and common size.
		MultiIndex(unsigned int dim, size_t size);

		// Dimension of the index
		unsigned int dim() const { return _dim; }

		// Value of the flat index, which numbers the multidimensional indices as they are ordered.
		size_t flat_index() const { return _flat; }

		// Total flat size
		size_t flat_size() const { return _flat_size; }

		// Indices (i1, i2, ..., iN)
		const std::vector<size_t>& indices() const { return _indices; }

		// Advance the index by 1 flat position
		MultiIndex& operator++();

		// Reset to initial state: indices == (0,0,0,0...) and flat_index() == 0
		void reset();

		// Set i-th index to new value.
		void set_index(size_t i, size_t value);

		// Set flat index to new value
		MultiIndex& operator=(size_t flat);

		static void decompose(size_t flat_index, size_t size, size_t dim, std::vector<size_t>& indices);

		static size_t flatten(size_t size, const std::vector<size_t>& indices);

		// One of the indices
		class Index {
		public:
			Index(MultiIndex* mi, size_t pos);
			Index(const Index& other) = default;
			Index& operator=(const Index& other) = delete;

			Index& operator++();

			// Compare with value
			bool operator!=(size_t value) const {
				return _mi->_indices[_pos] != value;
			}

			Index& operator=(size_t value);

			// Check if can be incremented safely
			bool has_next() const {
				return _mi->_indices[_pos] + 1 < _mi->_size;
			}
		private:
			const size_t _pos;
			MultiIndex* const _mi;
			size_t _step_size;
		};

		Index index(size_t pos);
	private:
		unsigned int _dim;
		size_t _size;
		size_t _flat_size;
		size_t _flat;
		std::vector<size_t> _indices;

		// Set i-th index to new value.
		void set_index(size_t i, size_t step_size, size_t value);
	};
}

#endif

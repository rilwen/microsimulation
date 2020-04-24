/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_MULTI_INDEX_MULTISIZE_H
#define __AVERISERA_MULTI_INDEX_MULTISIZE_H

#include <cstdlib>
#include <vector>

namespace averisera {
	// Class representing a multidimensional index with non-constant dimension. E.g. a 2D MultIndex represents a pair
	// (i1, i2) in which 0 <= i1 < N1 and 0 <= i2 < N2, with i1 changing faster.
	// Iteration example:
	// MultiIndexMultisize mi(2, {10, 20});
	// while (mi.flat_index() < mi.flat_size()) {
	//		// do something
	//		++mi;
	// }
	class MultiIndexMultisize {
	public:
        /** Default constructor */
        MultiIndexMultisize();

        /** Construct with same size */
        MultiIndexMultisize(unsigned int dim, size_t size)
            : MultiIndexMultisize(std::vector<size_t>(dim, size)) {
        }
        
		/** Construct with different sizes */
		MultiIndexMultisize(const std::vector<size_t>& sizes);

		/** Construct with different sizes - move constructor */
		MultiIndexMultisize(std::vector<size_t>&& sizes);

        /** Move assignment operator 
          Does nothing if &other == this
         */
        MultiIndexMultisize& operator=(MultiIndexMultisize&& other);

		/** Index dimension */
		unsigned int dim() const { return _dim; }

		// Value of the flat index, which numbers the multidimensional indices as they are ordered.
		size_t flat_index() const { return _flat; }

		// Total flat size
		size_t flat_size() const { return _flat_size; }

		// Indices (i1, i2, ..., iN)
		const std::vector<size_t>& indices() const { return _indices; }

        const std::vector<size_t>& sizes() const {
            return _sizes;
        }

		// Advance the index by 1 flat position
		MultiIndexMultisize& operator++();

		// Reset to initial state: indices == (0,0,0,0...) and flat_index() == 0
		void reset();

		// Set i-th index to new value.
		void set_index(size_t i, size_t value);

		// Set flat index to new value
		MultiIndexMultisize& operator=(size_t flat);

		static void decompose(size_t flat_index, const std::vector<size_t>& sizes, std::vector<size_t>& indices);

        /** Decompose flat index and cast results to other type 
          @tparam V index vector type
          @tparam T index type 
        */
        template <class T, class V> void decompose(size_t flat_index, V& indices) const {
            decompose_impl<T, V>(flat_index, _sizes, indices);
        }

		static size_t flatten(const std::vector<size_t>& sizes, const std::vector<size_t>& indices);

		// One of the indices
		class Index {
		public:
			Index(MultiIndexMultisize* mi, size_t pos);
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
				return _mi->_indices[_pos] + 1 < _mi->_sizes[_pos];
			}
		private:
			const size_t _pos;
			MultiIndexMultisize* const _mi;
			size_t _step_size;
		};

		Index index(size_t pos);
	private:
		unsigned int _dim;
		std::vector<size_t> _sizes;
		size_t _flat_size;
		size_t _flat;
		std::vector<size_t> _indices;

		// Set i-th index to new value.
		void set_index(size_t i, size_t step_size, size_t value);
        void validate() const;

        template <class T, class V> static void decompose_impl(size_t flat_index, const std::vector<size_t>& sizes, V& indices);
	};

    template <class T, class V> void MultiIndexMultisize::decompose_impl(size_t flat_index, const std::vector<size_t>& sizes, V& indices) {
        unsigned int i = 0;
		for (auto size_it = sizes.begin(); size_it != sizes.end(); ++i, ++size_it) {
            const size_t size = *size_it;
			const size_t a = flat_index / size;
			indices[i] = static_cast<T>(flat_index - a * size);
			flat_index = a;
		}
    }
}

#endif // __AVERISERA_MULTI_INDEX_MULTISIZE_H

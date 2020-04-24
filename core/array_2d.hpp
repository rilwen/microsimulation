/*
(C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_ARRAY_2D_H
#define __AVERISERA_ARRAY_2D_H

#include "vectors.hpp"

namespace averisera {
	//! Vector of vectors
	template <class T> class Array2D : public std::vector < std::vector<T> > {
	public:
	    using std::vector<std::vector<T>>::begin;
	    using std::vector<std::vector<T>>::end;

        typedef std::vector<T> row_t;

		Array2D(size_t rows = 0, size_t cols = 0)
			: std::vector<std::vector<T>>(rows)
		{
			ensure_region(0, 0, rows, cols);
		}

		Array2D(const Array2D<T>& other) = default;

        Array2D(Array2D<T>&& other)
            : std::vector<std::vector<T>>(std::move(other)) {
        }

        Array2D<T> & operator=(const Array2D<T>& other) {
            if (&other != this) {
                Array2D<T> copy(other);
                this->swap(copy);
            }
            return *this;
        }

        void swap(Array2D<T>& other) {
            std::vector<std::vector<T>>::swap(other);
        }

		// Construct from another container which has C::size()
		// and C[]::size()
		template <class C> static Array2D<T> from(const C& other)
		{
			const size_t n = other.size();
			Array2D<T> arr(n);
			for (size_t i = 0; i < n; ++i) {
				const auto& row = other[i];
				const size_t rn = row.size();
				auto& this_row = arr[i];
				this_row.resize(rn);
				for (size_t j = 0; j < rn; ++j) {
					this_row[j] = row[j];
				}
			}
			return arr;
		}

		// Ensure there is a row with index r
		void ensure_row(size_t r) {
			Vectors::ensure_elem(r, *this);
		}

		// Ensure there is an element with indices (r,c)
		void ensure_elem(size_t r, size_t c) {
			ensure_row(r);
			Vectors::ensure_elem(c, (*this)[r]);
		}

		// Ensure there is a 2D region with rows in the range [r0, r1) and columns in [c0, c1)
		// r1 >= r0
		// c1 >= c0
		void ensure_region(size_t r0, size_t c0, size_t r1, size_t c1) {
			Vectors::ensure_range(r0, r1, *this);
			for (size_t r = r0; r < r1; ++r) {
				Vectors::ensure_range(c0, c1, (*this)[r]);
			}
		}

		// Paste data from src at (dest_r, dest_c).
		void paste(const Array2D<T>& src, size_t dest_r, size_t dest_c) {
			const size_t r1 = dest_r + src.size();
			Vectors::ensure_range(dest_r, r1, *this);
			auto dst_r_it = begin() + dest_r;
			for (auto r_it = src.begin(); r_it != src.end(); ++r_it) {
				assert(dst_r_it != end());
				const size_t c1 = dest_c + r_it->size();
				Vectors::ensure_range(dest_c, c1, *dst_r_it);
				std::copy(r_it->begin(), r_it->end(), dst_r_it->begin() + dest_c);
				++dst_r_it;
			}
		}

		T& operator()(size_t r, size_t c) {
			ensure_elem(r, c);
			return (*this)[r][c];
		}

		const T& operator()(size_t r, size_t c) const {
			ensure_elem(r, c);
			return (*this)[r][c];
		}

		size_t max_row_size() const {
			size_t max_size = 0;
			for (auto it = begin(); it != end(); ++it) {
				max_size = std::max(max_size, it->size());
			}
			return max_size;
		}
	};

	template <class T> void swap(Array2D<T>& l, Array2D<T>& r) {
		l.swap(r);
	}
}

#endif

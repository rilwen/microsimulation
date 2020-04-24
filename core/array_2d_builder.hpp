/*
(C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_ARRAY_2D_BUILDER_H
#define __AVERISERA_ARRAY_2D_BUILDER_H

#include "array_2d.hpp"

namespace averisera {
	//! Builds a new Array2D object. Modifying functions return a reference to the builder to allow call chaining.
	template <class T> class Array2DBuilder {
	public:
		//! Construct empty builder
		Array2DBuilder();

		//! Add an element
		Array2DBuilder<T>& add(const T& elem);

		//! Add an array, reserving a minimal enclosing rectangle area for it.
		Array2DBuilder<T>& add(const Array2D<T>& other_arr);

		//! Move to next row
		Array2DBuilder<T>& new_row();

		//! Wipe out the array
		Array2DBuilder<T>& wipe();

		//! Return the constructed array
		Array2D<T> arr() const { return _arr; }
	private:
		Array2D<T> _arr;

		//! Index of the row where the insertions are now made
		size_t _curr_row;

		//! Index of the column where the next insertion would be made
		size_t _next_col;

		//! Index of the next row (not necessarily _curr_row + 1)
		size_t _next_row;
	};

	template <class T> Array2DBuilder<T>::Array2DBuilder()
		: _curr_row(0), _next_col(0), _next_row(1)
	{}

	template <class T> Array2DBuilder<T>& Array2DBuilder<T>::add(const T& elem) {
		_arr(_curr_row, _next_col) = elem;
		++_next_col;
		return *this;
	}

	template <class T> Array2DBuilder<T>& Array2DBuilder<T>::add(const Array2D<T>& other_arr) {
		const size_t width = other_arr.max_row_size();
		_arr.paste(other_arr, _curr_row, _next_col);
		_next_col += width;
		const size_t height = other_arr.size();
		_next_row = std::max(_next_row, _curr_row + height);
		return *this;
	}

	template <class T> Array2DBuilder<T>& Array2DBuilder<T>::new_row() {
		_curr_row = _next_row;
		_next_row = _curr_row + 1;
		_next_col = 0;
		return *this;
	}

	template <class T> Array2DBuilder<T>& Array2DBuilder<T>::wipe() {
		_arr = Array2D<T>();
		_curr_row = 0;
		_next_col = 0;
		_next_row = 0;
		return *this;
	}
}

#endif
/*
(C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_INDEX_ITERATOR_H
#define __AVERISERA_INDEX_ITERATOR_H

#include <cassert>
#include <cstddef>
#include <iterator>

namespace averisera {
	// Iterator over a vector class which supports the [] operator
	template <class T> class IndexIterator: public std::iterator<std::random_access_iterator_tag, typename std::remove_reference<decltype(((T*)nullptr)->operator[](0))>::type, ptrdiff_t> {
	public:		        
        /** Create empty iterator */
        IndexIterator() 
        : _vec(nullptr), _pos(static_cast<size_t>(-1))
        {
        }

        IndexIterator(T* vec, size_t pos)
			: _vec(vec)
			, _pos(pos)
		{
		}		

		IndexIterator(const IndexIterator<T>& other) = default;

        explicit operator bool() const {
            return _vec != nullptr;
        }

		IndexIterator<T>& operator=(const IndexIterator<T>& other) = default;

		IndexIterator<T>& operator++() {
			++_pos;
			return *this;
		}

		// post-increment
		IndexIterator<T> operator++(int) {
			IndexIterator<T> copy(*this);
			++_pos;
			return copy;
		}

		IndexIterator<T>& operator--() {
			--_pos;
			return *this;
		}

		// post-decrement
		IndexIterator<T> operator--(int) {
			IndexIterator<T> copy(*this);
			--_pos;
			return copy;
		}

		IndexIterator<T>& operator+=(ptrdiff_t delta) {
			_pos += delta;
			return *this;
		}

		IndexIterator<T>& operator-=(ptrdiff_t delta) {
			_pos -= delta;
			return *this;
		}

		IndexIterator<T> operator+(ptrdiff_t delta) const {
			return IndexIterator<T>(_vec, _pos + delta);
		}

		IndexIterator<T> operator-(ptrdiff_t delta) const {
			return IndexIterator<T>(_vec, _pos - delta);
		}

		ptrdiff_t operator-(const IndexIterator<T>& other) const {
			assert(_vec == other._vec);
			return _pos - other._pos;
		}

		bool operator!=(const IndexIterator<T>& other) const {
			assert(_vec == other._vec);
			return _pos != other._pos;
		}

		bool operator==(const IndexIterator<T>& other) const {
			assert(_vec == other._vec);
			return _pos == other._pos;
		}

		bool operator<(const IndexIterator<T>& other) const {
			assert(_vec == other._vec);
			return _pos < other._pos;
		}

		bool operator<=(const IndexIterator<T>& other) const {
			assert(_vec == other._vec);
			return _pos <= other._pos;
		}

		bool operator>(const IndexIterator<T>& other) const {
			assert(_vec == other._vec);
			return _pos < other._pos;
		}

		bool operator>=(const IndexIterator<T>& other) const {
			assert(_vec == other._vec);
			return _pos <= other._pos;
		}

		// Not "reference" to make iterating over Eigen Block objects work
		decltype(((T*)nullptr)->operator[](0)) operator*() const {
            assert(_vec != nullptr);
			return (*_vec)[_pos];
		}

		typename IndexIterator<T>::pointer operator->() const {
			assert(_vec != nullptr);
			return &(*_vec)[_pos];
		}

		// Special function
		size_t pos() const {
			return _pos;
		}
	private:
		// Pointer to indexed vector
		T* _vec;

		// Current position
		size_t _pos;
	};

	template <class T> IndexIterator<T> make_index_iterator(T& vec, size_t pos) {
		return IndexIterator<T>(&vec, pos);
            }

	template <class T> IndexIterator<T> make_index_iterator_begin(T& vec) {
		return make_index_iterator(vec, 0);
	}

	template <class T> IndexIterator<T> make_index_iterator_end(T& vec) {
		return make_index_iterator(vec, vec.size());
	}
}

#endif

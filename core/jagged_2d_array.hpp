/*
(C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_JAGGED_2D_ARRAY_H
#define __AVERISERA_JAGGED_2D_ARRAY_H

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <ostream>
#include <memory>
#include <boost/format.hpp>

namespace averisera {

	template <class T> class Jagged2DArray;
	template <class T> class Jagged2DArrayRowIter;

	template <class T>
	class Jagged2DArrayRowRef
	{
	public:
		Jagged2DArrayRowRef(const Jagged2DArrayRowRef<T>& other);
		Jagged2DArrayRowRef<T>& operator=(const Jagged2DArrayRowRef<T>&) = delete;

		// Empty reference.
		Jagged2DArrayRowRef();

		~Jagged2DArrayRowRef();

		size_t size() const;
		T& operator[](const size_t i);
		const T& operator[](const size_t i) const;

		bool operator==(const Jagged2DArrayRowRef<T>& other) const {
			if (this != &other) {
				if (size() == other.size()) {
					const auto end1 = end();
					auto it1 = begin();
					auto it2 = other.begin();
					while (it1 != end1) {
						if (*it1 != *it2) {
							return false;
						}
						++it1;
						++it2;
					}
					return true;
				} else {
					return false;
				}
			} else {
				return true;
			}
		}

		typedef T* iterator;
		typedef const T* const_iterator;
		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;

		/** Copy the reference data from other object */
		void reset(const Jagged2DArrayRowRef<T>& other);
		friend class Jagged2DArray<T>;
		friend class Jagged2DArrayRowIter < T > ;
	private:
		Jagged2DArrayRowRef(T* const data, const size_t size);
		T* data_;
		size_t size_;
	};

	template <class T>
	Jagged2DArrayRowRef<T>::Jagged2DArrayRowRef(T* const data, const size_t size)
	: data_(data), size_(size)
	{
	}

	template <class T>
	Jagged2DArrayRowRef<T>::Jagged2DArrayRowRef()
	: data_(0), size_(0)
	{
	}

	template <class T>
	Jagged2DArrayRowRef<T>::Jagged2DArrayRowRef(const Jagged2DArrayRowRef<T>& other)
	: data_(other.data_), size_(other.size_)
	{
	}

	template <class T>
	Jagged2DArrayRowRef<T>::~Jagged2DArrayRowRef()
	{
	}

	template <class T>
	inline size_t Jagged2DArrayRowRef<T>::size() const
	{
		return size_;
	}

	template <class T>
	inline T& Jagged2DArrayRowRef<T>::operator[](const size_t i)
	{
	#ifndef NDEBUG
		if (i >= size_) {
			throw std::out_of_range((boost::format("Row element index out of range: %d") % i).str());
		}
	#endif
		return data_[i];
	}

	template <class T>
	inline const T& Jagged2DArrayRowRef<T>::operator[](const size_t i) const
	{
	#ifndef NDEBUG
		if (i >= size_) {
			throw std::out_of_range((boost::format("Row element index out of range: %d") % i).str());
		}
	#endif
		return data_[i];
	}

	template <class T>
	inline T* Jagged2DArrayRowRef<T>::begin()
	{
		return data_;
	}

	template <class T>
	inline T* Jagged2DArrayRowRef<T>::end()
	{
		return data_ + size_;
	}

	template <class T>
	inline const T* Jagged2DArrayRowRef<T>::begin() const
	{
		return data_;
	}

	template <class T>
	inline const T* Jagged2DArrayRowRef<T>::end() const
	{
		return data_ + size_;
	}

	template <class T>
	void Jagged2DArrayRowRef<T>::reset(const Jagged2DArrayRowRef<T>& other)
	{
		data_ = other.data_;
		size_ = other.size_;
	}

	template <class T>
	class Jagged2DArrayRowIter {
	public:
		typedef std::forward_iterator_tag iterator_category;
		typedef Jagged2DArrayRowRef<T> value_type;
		typedef ptrdiff_t difference_type;
		typedef value_type* pointer;
		typedef value_type& reference;

		Jagged2DArrayRowIter(T** row_start)
			: row_start_(row_start)
		{}

		Jagged2DArrayRowRef<T> operator*() {
			T* const data = *row_start_;
			return Jagged2DArrayRowRef<T>(data, *(row_start_ + 1) - data);
		}

		const Jagged2DArrayRowRef<T> operator*() const {
			const T* const data = *row_start_;
			return Jagged2DArrayRowRef<T>(data, *(row_start_ + 1) - data);
		}

		bool operator!=(const Jagged2DArrayRowIter<T>& other) const {
			return row_start_ != other.row_start_;
		}

		bool operator==(const Jagged2DArrayRowIter<T>& other) const {
			return row_start_ == other.row_start_;
		}

		Jagged2DArrayRowIter<T>& operator++() {
			++row_start_;
			return *this;
		}
	private:
		T** row_start_;
	};

    /** 2D array with continuous memory layout which can hold rows of different size.
     */
	template <class T> class Jagged2DArray {
	public:
        /** Construct a rectangular array and initialize elements to a constant value */
		Jagged2DArray(const size_t nbr_rows, const size_t nbr_cols, const T& elem );

        /** Construct a rectangular array without initialization of elements */
		Jagged2DArray(const size_t nbr_rows, const size_t nbr_cols);

        /** Default constructor */
		Jagged2DArray();

        /** Copy constructor */
		Jagged2DArray(const Jagged2DArray<T>& other);

        /** Move constructor */
		Jagged2DArray(Jagged2DArray<T>&& other);

		/**
		Copy the data from the old container.
		@tparam C 2D container with begin(), end() and size() on each level.
		template <class C>
		*/
		template <class C>
		Jagged2DArray(const C& data);

		~Jagged2DArray();

		/* Factory methods */

		//! Create a lower triangular matrix
		//! @param N number rows
		//! @param include_diagonal Does the matrix include diagonal entries (i == j) or just those below the diagonal (i > j)?
		static Jagged2DArray<T> lower_triangular(const size_t N, const bool include_diagonal);
		static Jagged2DArray<T> lower_triangular(const size_t N, const bool include_diagonal, const T& elem);

		/**
		* Copy the row sizes from the iterator.
		* @tparam Iter Const Iterator.
		*/
		template <class Iter>
		static Jagged2DArray<T> from_iters(Iter begin, Iter end);

		/** Does not throw */
		void swap(Jagged2DArray<T>& other);

		Jagged2DArray<T>& operator=(const Jagged2DArray<T>& other);

		/** Number of rows */
		size_t size() const;

		/** Checks indices in debug mode
		 * @throws std::out_of_range if index out of range
		*/
		size_t row_size(const size_t row) const;

		/** Total number of elements */
		size_t nbr_elements() const;

		/** Checks indices in debug mode
		 * @throws std::out_of_range if index out of range
		*/
		T& operator()(const size_t row, const size_t row_elem);
		/** Checks indices in debug mode
		 * @throws std::out_of_range if index out of range
		*/
		const T& operator()(const size_t row, const size_t row_elem) const;

		//! Fill with constant element
		void fill(const T& elem);

		typedef Jagged2DArrayRowRef < T > row_t;

		row_t operator[](const size_t row_idx);
		const row_t operator[](const size_t row_idx) const;

		row_t flat_form();
		const row_t flat_form() const;
		typedef typename Jagged2DArrayRowRef<T>::iterator flat_iterator;
		flat_iterator flat_begin();
		flat_iterator flat_end();
		typedef typename Jagged2DArrayRowRef<T>::const_iterator const_flat_iterator;
		const_flat_iterator flat_begin() const;
		const_flat_iterator flat_end() const;

		typedef Jagged2DArrayRowIter<T> row_iterator;
		row_iterator row_begin() {
			return Jagged2DArrayRowIter<T>(row_starts_.get());
		}
		const row_iterator row_begin() const {
			return Jagged2DArrayRowIter<T>(row_starts_.get());
		}
		row_iterator row_end() {
			return Jagged2DArrayRowIter<T>(row_starts_.get() + size_);
		}
		const row_iterator row_end() const {
			return Jagged2DArrayRowIter<T>(row_starts_.get() + size_);
		}

		bool operator==(const Jagged2DArray<T>& other) const {
			if (this == &other) {
				return true;
			}
			if (size_ != other.size_) {
				return false;
			}
			for (size_t i = 0; i < size_; ++i) {
				if (row_size(i) != other.row_size(i)) {
					return false;
				}
			}
			return std::equal(flat_begin(), flat_end(), other.flat_begin());
		}

		bool operator!=(const Jagged2DArray<T>& other) const {
			return !(*this == other);
		}

		// friends
		friend Jagged2DArray<T>& operator+=(Jagged2DArray<T>& left, const Jagged2DArray<T>& right) {
		  assert(left.size_ == right.size_);
		  const T* right_ptr = right.data_.get();
		  T* const left_end = left.data_.get() + left.size_;
		  for (T* it = left.data_.get(); it != left_end; ++it) {
			  assert( right_ptr != right.data_.get() + right.size_ );
			  *it += *right_ptr;
			  ++right_ptr;
		  }
		  return left;
		}

		friend Jagged2DArray<T>& operator-=(Jagged2DArray<T>& left, const Jagged2DArray<T>& right) {
		  assert(left.size_ == right.size_);
		  const T* right_ptr = right.data_.get();
		  T* const left_end = left.data_.get() + left.size_;
		  for (T* it = left.data_.get(); it != left_end; ++it) {
			  assert( right_ptr != right.data_.get() + right.size_ );
			  *it -= *right_ptr;
			  ++right_ptr;
		  }
		  return left;
		}

		friend Jagged2DArray<T>& operator*=(Jagged2DArray<T>& left, const T& right)
		{
		  T* const left_end = left.data_.get() + left.size_;
		  for (T* it = left.data_.get(); it != left_end; ++it) {
			  *it *= right;
		  }
		  return left;
		}

		friend Jagged2DArray<T>& operator/=(Jagged2DArray<T>& left, const T& right)
		{
		  T* const left_end = left.data_.get() + left.size_;
		  for (T* it = left.data_.get(); it != left_end; ++it) {
			  *it /= right;
		  }
		  return left;
		}

		/*friend std::ostream& operator<<(std::ostream& os, const Jagged2DArray<T>& arr) {
			os << "[";
			for (size_t i = 0; i < arr.size(); ++i) {
				const size_t rowlen = arr.row_size(i);
				os << "[";
				const auto row = arr[i];
				for (size_t j = 0; j < rowlen; ++j) {
					os << row[j];
					if (j + 1 < rowlen) {
						os << ", ";
					}
				}
				os << "]";
				if (i + 1 < arr.size()) {
					os << ",\n";
				}
			}
			os << "]";
			return os;
		}*/

		// Init to a rectangular shape
		void init_rectangular(const size_t nbr_rows, const size_t nbr_cols);

		Jagged2DArray<T> row_range(size_t begin_idx, size_t nrows) const;
	private:
		void init_triangular(const size_t nbr_rows, const bool include_diagonal);
		static size_t triangular_size(size_t N, bool include_diagonal);

		/** Number of rows */
		size_t size_;

		std::unique_ptr<T*[]> row_starts_;
		std::unique_ptr<T[]> data_;
	};

	template <class T>
	Jagged2DArray<T>::Jagged2DArray(const size_t nbr_rows, const size_t nbr_cols, const T& elem)
	{
		init_rectangular(nbr_rows, nbr_cols);
		fill(elem);
	}

	template <class T>
	Jagged2DArray<T>::Jagged2DArray(const size_t nbr_rows, const size_t nbr_cols)
	{
		init_rectangular(nbr_rows, nbr_cols);
	}

	template <class T>
	Jagged2DArray<T>::Jagged2DArray()
	{
		init_rectangular(0u, 0u);
	}

	template <class T>
	Jagged2DArray<T>::Jagged2DArray(const Jagged2DArray<T>& other)
	: size_(other.size_), row_starts_(new T*[other.size_ + 1]), data_(new T[other.nbr_elements()])
	{
		std::copy(other.data_.get(), other.data_.get() + other.nbr_elements(), data_.get());
		row_starts_[0] = data_.get();
		for (size_t i = 0; i < size_; ++i) {
			row_starts_[i + 1] = row_starts_[i] + other.row_size(i);
		}
	}

	template <class T>
	Jagged2DArray<T>::Jagged2DArray(Jagged2DArray<T>&& other)
		: size_(other.size_), row_starts_(std::move(other.row_starts_)), data_(std::move(other.data_))
	{
		other.size_ = 0;
	}

	template <class T>
	template <class Iter>
	Jagged2DArray<T> Jagged2DArray<T>::from_iters(Iter begin, Iter end)
	{
		Jagged2DArray<T> result;
		// count rows
		result.size_ = 0;
		size_t total_data_size = 0;
		for (Iter i = begin; i != end; ++i) {
			++result.size_;
			total_data_size += *i;
		}
		result.row_starts_.reset(new T*[result.size_ + 1]);
		result.data_.reset(new T[total_data_size]);
		if (total_data_size > 0) {
			assert(result.size_);
			T* current_data_ptr = result.data_.get();
			T** current_row_start = result.row_starts_.get();
			for (Iter i = begin; i != end; ++i) {
				size_t row_size = *i;
				*current_row_start = current_data_ptr;
				current_data_ptr += row_size;
				++current_row_start;
			}
			*current_row_start = current_data_ptr;
		} else {
			std::fill(result.row_starts_.get(), result.row_starts_.get() + result.size_ + 1, nullptr);
		}
		return result;
	}

	template <class T>
	template <class C>
	Jagged2DArray<T>::Jagged2DArray(const C& data)
	: size_(data.size()), row_starts_(new T*[size_ + 1]), data_(nullptr)
	{
		size_t total_data_size = 0;
		for (auto i = data.begin(); i != data.end(); ++i) {
			total_data_size += i->size();
		}
		data_.reset(new T[total_data_size]);
		if (total_data_size > 0) {
			assert(size_);
			T* current_data_ptr = data_.get();
			T** current_row_start = row_starts_.get();
			for (auto i = data.begin(); i != data.end(); ++i) {
				*current_row_start = current_data_ptr;
				std::copy(i->begin(), i->end(), current_data_ptr);
				++current_row_start;
				current_data_ptr += i->size();
			}
			*current_row_start = current_data_ptr;
		} else {
			std::fill(row_starts_.get(), row_starts_.get() + size_ + 1, nullptr);
		}
	}

	template <class T>
	Jagged2DArray<T>::~Jagged2DArray()
	{
	}

	template <class T>
	Jagged2DArray<T> Jagged2DArray<T>::lower_triangular(const size_t N, const bool include_diagonal)
	{
		Jagged2DArray<T> result;
		result.init_triangular(N, include_diagonal);
		return result;
	}

	template <class T>
	Jagged2DArray<T> Jagged2DArray<T>::lower_triangular(const size_t N, const bool include_diagonal, const T& elem)
	{
		Jagged2DArray<T> result(Jagged2DArray<T>::lower_triangular(N, include_diagonal));
		result.fill(elem);
		return result;
	}

	template <class T>
	void Jagged2DArray<T>::init_rectangular(const size_t nbr_rows, const size_t nbr_cols)
	{
		size_ = nbr_rows;
		row_starts_.reset(new T*[nbr_rows + 1]);
		data_.reset(new T[nbr_rows * nbr_cols]);
		// remember that row_starts_ has 1 element more than we have rows
		T* current_data_ptr = data_.get();
		T** current_row_start = row_starts_.get();
		for (size_t i = 0; i <= size_; ++i) {
			*current_row_start = current_data_ptr;
			++current_row_start;
			current_data_ptr += nbr_cols;
		}
	}

	template <class T>
	void Jagged2DArray<T>::init_triangular(const size_t nbr_rows, const bool include_diagonal)
	{
		size_ = nbr_rows;
		row_starts_.reset(new T*[nbr_rows + 1]);
		data_.reset(new T[triangular_size(nbr_rows, include_diagonal)]);
		T** current_row_start = row_starts_.get();
		T* current_data_ptr = data_.get();
		for (size_t i = 0; i <= size_; ++i) {
			*current_row_start = current_data_ptr;
			current_data_ptr += include_diagonal ? (i + 1) : i;
			++current_row_start;
		}
	}

	template <class T>
	size_t Jagged2DArray<T>::triangular_size(size_t N, bool include_diagonal)
	{
		return include_diagonal ? (N*(N+1))/2 : (N*(N-1))/2;
	}

	template <class T>
	Jagged2DArray<T>& Jagged2DArray<T>::operator=(const Jagged2DArray<T>& other)
	{
		// check for self-assingment
		if (this != &other) {
			// use copy-and-swap technique
			Jagged2DArray<T> copy(other);
			size_ = copy.size_;
			row_starts_.swap(copy.row_starts_);
			data_.swap(copy.data_);
		}
		return *this;
	}

	template <class T>
	void Jagged2DArray<T>::swap(Jagged2DArray<T>& other)
	{
		if (this != &other) {
			using std::swap;
			data_.swap(other.data_);
			row_starts_.swap(other.row_starts_);
			swap(size_, other.size_);
		}
	}

	template <class T>
	inline size_t Jagged2DArray<T>::size() const
	{
		return size_;
	}

	// Implementation detail
	inline std::string ja_err_msg_row_(const size_t row)
	{
		return (boost::format("Row index out of range: %d") % row).str();		
	}

	template <class T>
	inline size_t Jagged2DArray<T>::row_size(const size_t row) const
	{
	#ifndef NDEBUG
		if (row >= size_) {
			throw std::out_of_range(ja_err_msg_row_(row));
		}
	#endif
		return static_cast<size_t>(row_starts_[row + 1] - row_starts_[row]);
	}

	template <class T>
	inline size_t Jagged2DArray<T>::nbr_elements() const
	{
		return row_starts_[size_] - row_starts_[0];
	}

	template <class T>
	inline T& Jagged2DArray<T>::operator()(const size_t row, const size_t row_elem)
	{
	#ifndef NDEBUG
		if (row >= size_) {
			throw std::out_of_range(ja_err_msg_row_(row));
		}
		if (row_elem >= row_size(row)) {
			throw std::out_of_range((boost::format("Row element index out of range: %d") % row_elem).str());
		}
	#endif
		return *(row_starts_[row] + row_elem);
	}

	template <class T>
	inline const T& Jagged2DArray<T>::operator()(const size_t row, const size_t row_elem) const
	{
	#ifndef NDEBUG
		if (row >= size_) {
			throw std::out_of_range(ja_err_msg_row_(row));
		}
		if (row_elem >= row_size(row)) {
			throw std::out_of_range((boost::format("Row element index out of range: %d") % row_elem).str());
		}
	#endif
		return *(row_starts_[row] + row_elem);
	}

	template <class T>
	inline Jagged2DArrayRowRef<T> Jagged2DArray<T>::operator[](const size_t row_idx)
	{
	#ifndef NDEBUG
		if (row_idx >= size_) {
			throw std::out_of_range(ja_err_msg_row_(row_idx));
		}
	#endif
		return Jagged2DArrayRowRef<T>(row_starts_[row_idx], row_size(row_idx));
	}

	template <class T>
	inline const Jagged2DArrayRowRef<T> Jagged2DArray<T>::operator[](const size_t row_idx) const
	{
	#ifndef NDEBUG
		if (row_idx >= size_) {
			throw std::out_of_range(ja_err_msg_row_(row_idx));
		}
	#endif
		return Jagged2DArrayRowRef<T>(row_starts_[row_idx], row_size(row_idx));
	}

	template <class T>
	void Jagged2DArray<T>::fill(const T& elem)
	{
		std::fill(data_.get(), data_.get() + nbr_elements(), elem);
	}

	template <class T>
	inline Jagged2DArrayRowRef<T> Jagged2DArray<T>::flat_form()
	{
		return Jagged2DArrayRowRef<T>(data_.get(), nbr_elements());
	}

	template <class T>
	inline const Jagged2DArrayRowRef<T> Jagged2DArray<T>::flat_form() const
	{
		return Jagged2DArrayRowRef<T>(data_.get(), nbr_elements());
	}

	template <class T>
	inline typename Jagged2DArrayRowRef<T>::iterator Jagged2DArray<T>::flat_begin()
	{
		return data_.get();
	}

	template <class T>
	inline typename Jagged2DArrayRowRef<T>::iterator Jagged2DArray<T>::flat_end()
	{
		return data_.get() + nbr_elements();
	}

	template <class T>
	inline typename Jagged2DArrayRowRef<T>::const_iterator Jagged2DArray<T>::flat_begin() const
	{
		return data_.get();
	}

	template <class T>
	inline typename Jagged2DArrayRowRef<T>::const_iterator Jagged2DArray<T>::flat_end() const
	{
		return data_.get() + nbr_elements();
	}

	template <class T>
	Jagged2DArray<T> Jagged2DArray<T>::row_range(const size_t begin_idx, const size_t nrows) const {
		assert(begin_idx < size_);
		const size_t end_idx = begin_idx + nrows;
		assert(end_idx <= size_);
		Jagged2DArray<T> copy;
		copy.size_ = nrows;
		copy.row_starts_.reset(new T*[nrows + 1]);
		const T* const data_begin = row_starts_[begin_idx];
		const T* const data_end = row_starts_[end_idx];
		const size_t data_size = data_end - data_begin;
		copy.data_.reset(new T[data_size]);
		std::copy(data_begin, data_end, copy.data_.get());
		for (size_t i = 0; i <= nrows; ++i) {
			copy.row_starts_[i] = copy.data_.get() + (row_starts_[begin_idx + i] - row_starts_[begin_idx]);
		}
		return copy;
	}

	// Free functions for jagged arrays

	/** Does not throw */
	template <class T>
	void swap(Jagged2DArray<T>& a, Jagged2DArray<T>& b)
	{
		a.swap(b);
	}

	/** If it fails, it does not change the input object */
	template <class T>
	void resize(Jagged2DArray<T>& a, size_t nbr_rows, size_t nbr_cols)
	{
		Jagged2DArray<T> nowy(nbr_rows, nbr_cols);
		a.swap(nowy);
	}

	/** If it fails, it does not change the input object. Caller must provide iterators begin() and end()
	with new row sizes.
	@tparam Iter const iterator
	*/
	template <class T, class Iter>
	void resize(Jagged2DArray<T>& a, Iter begin, Iter end)
	{
		Jagged2DArray<T> nowy(begin, end);
		a.swap(nowy);
	}

	template <class T>
	void print(const Jagged2DArray<T>& array, std::ostream& out)
	{
		out << "Nbr rows: " << array.size() << std::endl;
		for (size_t i = 0; i < array.size(); ++i) {
			assert( array.row_size(i) == array[i].size() );
			out << "[" << array.row_size(i) << "] ";
			for (size_t j = 0; j < array.row_size(i); ++j) {
				out << array(i, j) << " ";
			}
			out << std::endl;
		}
	}
}

#endif

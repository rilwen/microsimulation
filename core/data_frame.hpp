// (C) Averisera Ltd 2014-2020
#pragma once
#include "csv_file_reader.hpp"
#include "eigen.hpp"
#include "padding.hpp"
#include "preconditions.hpp"
#include <Eigen/Core>
#include <cassert>
#include <cstdint>
#include <iosfwd>
#include <limits>
#include <utility>
#include <vector>

namespace averisera {
	/**
	Wrapper around a Matrix and set of row/column labels.	
	*/
	template <class C, class I, class V = double> class DataFrame {
	public:
		typedef C column_label_type;
		typedef I index_label_type;
		typedef V element_type;
		typedef size_t size_type;
		typedef Eigen::Matrix<V, Eigen::Dynamic, Eigen::Dynamic> values_type;
		typedef std::vector<C> columns_type;
		typedef std::vector<I> index_type;

		DataFrame() 
			: nbr_rows_(0), nbr_cols_(0)
		{}

		DataFrame(const values_type& values, const columns_type& columns, const index_type& index)
			: values_(values), columns_(columns), index_(index), nbr_rows_(index.size()), nbr_cols_(columns.size()) {
			check_equals(values.rows(), index.size(), "DataFrame");
			check_equals(values.cols(), columns.size(), "DataFrame");
		}

		DataFrame(const columns_type& columns, const index_type& index)
			: values_(index.size(), columns.size()), columns_(columns), index_(index), nbr_rows_(index.size()), nbr_cols_(columns.size()) {
		}

		DataFrame(V value, const columns_type& columns, const index_type& index)
			: values_(index.size(), columns.size()), columns_(columns), index_(index), nbr_rows_(index.size()), nbr_cols_(columns.size()) {
			values_.setConstant(value);
		}

		DataFrame(values_type&& values, columns_type&& columns, index_type&& index)
			: nbr_rows_(index.size()), nbr_cols_(columns.size()) {
			check_equals(values.rows(), index.size(), "DataFrame");
			check_equals(values.cols(), columns.size(), "DataFrame");
			values_ = std::move(values);
			values.resize(0, 0); // Eigen doesn't do it after move
			columns_ = std::move(columns);
			index_ = std::move(index);
		}

		DataFrame(columns_type&& columns, index_type&& index)
			: values_(index.size(), columns.size()), nbr_rows_(index.size()), nbr_cols_(columns.size()) {
			columns_ = std::move(columns);
			index_ = std::move(index);
		}

		DataFrame(V value, columns_type&& columns, index_type&& index)
			: values_(index.size(), columns.size()), nbr_rows_(index.size()), nbr_cols_(columns.size()) {
			columns_ = std::move(columns);
			index_ = std::move(index);
			values_.setConstant(value);
		}

		DataFrame(DataFrame<C, I, V>&& other) noexcept
			: values_(std::move(other.values_)),
			columns_(std::move(other.columns_)),
			index_(std::move(other.index_)),
			nbr_rows_(other.nbr_rows_),
			nbr_cols_(other.nbr_cols_) {
			other.values_.resize(0, 0); // Eigen doesn't do it after move
		}

		template <class C2, class I2> DataFrame(DataFrame<C2, I2, V>&& other, columns_type&& columns, index_type&& index)
		: nbr_rows_(index.size()), nbr_cols_(columns.size()) {
			check_equals(other.nbr_rows(), index.size(), "DataFrame");
			check_equals(other.nbr_cols(), columns.size(), "DataFrame");
			DataFrame<C2, I2, V> moved(std::move(other)); // to wipe out other
			columns_ = std::move(columns);
			index_ = std::move(index);
			values_ = std::move(moved.values_);
			moved.values_.resize(0, 0); // Eigen doesn't do it after move
		}

		DataFrame(const DataFrame<C, I, V>& other)
			: values_(other.values_),
			columns_(other.columns_),
			index_(other.index_),
			nbr_rows_(other.nbr_rows_),
			nbr_cols_(other.nbr_cols_) {
		}

		DataFrame<C, I, V>& operator=(DataFrame<C, I, V>&& other) noexcept {
			values_ = std::move(other.values_);
			columns_ = std::move(other.columns_);
			index_ = std::move(other.index_);
			nbr_rows_ = other.nbr_rows_;
			nbr_cols_ = other.nbr_cols_;
			other.nbr_rows_ = 0;
			other.nbr_cols_ = 0;
			other.values_.resize(0, 0); // Eigen doesn't do it after move
			return *this;
		}

		DataFrame<C, I, V>& operator=(const DataFrame<C, I, V>& other) {
			DataFrame<C, I, V> copy(other);
			this->swap(copy);
			return *this;
		}

		bool operator==(const DataFrame<C, I, V>& other) const {
			return values_ == other.values_ &&
				columns_ == other.columns_ &&
				index_ == other.index_;
		}

		/** Column labels */
		const columns_type& columns() const {
			return columns_;
		}

		/** Index */
		const index_type& index() const {
			return index_;
		}

		/** Values */
		values_type& values() {
			return values_;
		}

		/** Values */
		const values_type& values() const {
			return values_;
		}

		/** Number of rows */
		size_type nbr_rows() const {
			return nbr_rows_;
		}

		/** Number of columns */
		size_type nbr_cols() const {
			return nbr_cols_;
		}

		/** Pair of (number of rows, number of columns) */
		std::pair<size_type, size_type> shape() const {
			return std::make_pair(nbr_rows(), nbr_cols());
		}

		/** Return value from r-th row and c-th column. */
		V ix(size_type r, size_type c) const {
			assert(r < nbr_rows());
			assert(c < nbr_cols());
			return values_(r, c);
		}

		/** Return reference to value from r-th row and c-th column. */
		V& ix(size_type r, size_type c) {
			assert(r < nbr_rows());
			assert(c < nbr_cols());
			return values_(r, c);
		}

		/** Return value with index label row and column label col. */
		V loc(I row, C col) const {
			return ix(row_idx(row), col_idx(col));
		}

		/** Return reference to value with index label row and column label col. */
		V& loc(I row, C col) {
			return ix(row_idx(row), col_idx(col));
		}

		/** Integer-index column */
		typename values_type::ConstColXpr col_values_ix(size_type c) const {
			assert(c < nbr_cols());
			return values_.col(c);
		}

		/** Integer-index column */
		typename values_type::ColXpr col_values_ix(size_type c) {
			assert(c < nbr_cols());
			return values_.col(c);
		}

		/** Column with given label */
		typename values_type::ConstColXpr col_values(const C& label) const {
			return col_values_ix(col_idx(label));
		}

		/** Column with given label */
		typename values_type::ColXpr col_values(const C& label) {
			return col_values_ix(col_idx(label));
		}

		/** Check if columns contain this label */
		bool has_col_label(const C& label) const {
			const auto it = std::find(columns_.begin(), columns_.end(), label);
			return it != columns_.end();
		}

		/** Integer-index row */
		typename values_type::ConstRowXpr row_values_ix(size_type r) const {
			assert(r < nbr_rows());
			return values_.row(r);
		}

		typename values_type::ConstRowXpr row_values(const I& label) const {
			return row_values_ix(row_idx(label));
		}

		typename values_type::RowXpr row_values(const I& label) {
			return row_values_ix(row_idx(label));
		}

		size_type col_idx(const C& label) const {
			return find_idx_safe(columns_, label);
		}

		size_type row_idx(const I& label) const {
			return find_idx_safe(index_, label);
		}

		/** Return NOT_FOUND if label not found */
		size_type col_idx_unsafe(const C& label) const {
			return find_idx_unsafe(columns_, label);
		}

		/** Return NOT_FOUND if label not found */
		size_type row_idx_unsafe(const I& label) const {
			return find_idx_unsafe(index_, label);
		}

		/** Return a copy of the DataFrame with row "label" removed. */
		DataFrame<C, I, V> drop_row(const I& label) const {
			size_type idx;
			try {
				idx = row_idx(label);
				return drop_row_ix(idx);
			} catch (std::out_of_range&) {
				return DataFrame<C, I, V>(*this);
			}
		}

		/** Return a copy of the DataFrame with idx-th row removed. */
		DataFrame<C, I, V> drop_row_ix(size_type idx) const {
			assert(idx < nbr_rows());
			size_type new_nbr_rows = nbr_rows() - 1;
			values_type new_values(new_nbr_rows, nbr_cols());
			index_type new_index(new_nbr_rows);
			new_values.block(0, 0, idx, nbr_cols()) = values_.block(0, 0, idx, nbr_cols());
			new_values.block(idx, 0, new_nbr_rows - idx, nbr_cols()) = values_.block(idx + 1, 0, new_nbr_rows - idx, nbr_cols());
			std::copy(index_.begin(), index_.begin() + idx, new_index.begin());
			std::copy(index_.begin() + idx + 1, index_.end(), new_index.begin() + idx);
			return DataFrame<C, I, V>(new_values, columns_, new_index);
		}

		DataFrame<C, I, V>& operator/=(V x) {
			values_ /= x;
			return *this;
		}

		DataFrame<C, I, V> operator/(V x) const {
			DataFrame<C, I, V> copy(*this);
			copy /= x;
			return copy;
		}

		DataFrame<C, I, V>& operator*=(V x) {
			values_ *= x;
			return *this;
		}

		DataFrame<C, I, V> operator*(V x) const {
			DataFrame<C, I, V> copy(*this);
			copy *= x;
			return copy;
		}

		DataFrame<C, I, V> operator-(const DataFrame<C, I, V>& other) const {
			check_equals(index(), other.index(), "DataFrame: indices must be equal");
			check_equals(columns(), other.columns(), "DataFrame: columns must be equal");
			return DataFrame<C, I, V>(values_ - other.values_, columns(), index());
		}

		DataFrame<C, I, V> operator+(const DataFrame<C, I, V>& other) const {
			check_equals(index(), other.index(), "DataFrame: indices must be equal");
			check_equals(columns(), other.columns(), "DataFrame: columns must be equal");
			return DataFrame<C, I, V>(values_ + other.values_, columns(), index());
		}

		DataFrame<C, I, V>& operator+=(const DataFrame<C, I, V>& other) {
			check_equals(index(), other.index(), "DataFrame: indices must be equal");
			check_equals(columns(), other.columns(), "DataFrame: columns must be equal");
			values_ += other.values_;
			return *this;
		}

		void set_column_label(size_type index, const C& new_label) {
			assert(index < nbr_cols());
			// we could optionally re-sort the labels to ensure quick search...
			columns_[index] = new_label;
		}

		bool empty() const {
			return nbr_cols_ == 0 && nbr_rows_ == 0;
		}

		/** Sort index entries (and data) */
		DataFrame<C, I, V>& sort_index() {
			typedef std::pair<I, size_type> kvpair;
			std::vector<kvpair> kv(nbr_rows());
			size_type i = 0;
			for (const I& il : index_) {
				kv[i] = std::make_pair(il, i);
				++i;
			}
			sort_rows_by_kvpairs(kv);
			return *this;
		}

		/** Sort column entries (and data) */
		DataFrame<C, I, V>& sort_columns() {
			typedef std::pair<C, size_type> kvpair;
			std::vector<kvpair> kv(nbr_cols());
			size_type i = 0;
			for (const C& cl : columns_) {
				kv[i] = std::make_pair(cl, i);
				++i;
			}
			std::sort(kv.begin(), kv.end(), [](const kvpair& l, const kvpair& r) { return l.first < r.first; });
			values_type new_values(nbr_rows(), nbr_cols());
			columns_type new_columns(nbr_cols());
			for (i = 0; i < nbr_cols(); ++i) {
				new_columns[i] = kv[i].first;
				new_values.col(i) = values_.col(kv[i].second);
			}
			values_.swap(new_values);
			columns_.swap(new_columns);
			return *this;
		}

		/** Sort index and data by the values in sorted_column, in ascending order. */
		DataFrame<C, I, V>& sort_values(const C& sorted_column) {
			return sort_values(sorted_column, true);
		}

		/** Sort index and data by the values in sorted_column, in ascending
		or descending order. */
		DataFrame<C, I, V>& sort_values(const C& sorted_column, bool ascending) {
			auto identity = [](const V& x) { return x; };
			return sort_values(sorted_column, ascending, identity);
		}

		/** Sort index and data by the function of the values in sorted_column, in ascending 
		or descending order. */
		template <class F> DataFrame<C, I, V>& sort_values(const C& sorted_column, bool ascending, F function) {
			check_that(has_col_label(sorted_column), "DataFrame::sort_values: column present");
			typedef std::pair<V, size_type> kvpair;
			std::vector<kvpair> kv(nbr_rows());
			const auto sorted_column_index = col_idx(sorted_column);
			for (size_t i = 0; i < nbr_rows(); ++i) {
				kv[i] = std::make_pair(function(values()(i, sorted_column_index)), i);
			}			
			sort_rows_by_kvpairs(kv, ascending);
			return *this;
		}

		typedef V* value_iterator;
		typedef const V* value_const_iterator;

		value_const_iterator values_begin() const {
			return values_.data();
		}

		value_const_iterator values_end() const {
			return values_.data() + nbr_cols_ * nbr_rows_;
		}

		void swap(DataFrame<C, I, V>& other) noexcept {
			columns_.swap(other.columns_);
			index_.swap(other.index_);
			values_.swap(other.values_);
			std::swap(nbr_cols_, other.nbr_cols_);
			std::swap(nbr_rows_, other.nbr_rows_);
		}

		/** Has to be 0! */
		static const size_type INDEX_COLUMN_POSITION = 0;

		/**
		@param pad_nan_columns If true, pad NaN values. */
		template <class CR, class IR> static DataFrame<C, I, V> from_csv_file(CSVFileReader& reader, CR column_converter, IR index_converter, const bool use_nans_for_missing, const bool pad_nan_columns) {
			const std::vector<std::string> col_names(reader.read_column_names());
			if (col_names.size() < 1) {
				throw DataException(boost::str(boost::format("DataFrame: not enough columns in file %s") % reader.file_name()));
			}
			/*const auto missing_year_handler = [&reader, index_column_position](const std::exception&, const std::string& elem) {
				throw DataException(boost::str(boost::format("DataFrame: cannot convert \"%s\" to year number in file %s, column %d") % elem % reader.file_name() % index_column_position));
				return 0;
			};*/

			std::vector<I> index(reader.count_data_rows());
			std::copy(reader.begin_converting<I>(INDEX_COLUMN_POSITION, index_converter), reader.end_converting<I>(INDEX_COLUMN_POSITION, index_converter), index.begin());

			std::vector<C> columns(col_names.size() - 1);
			std::transform(col_names.begin() + 1, col_names.end(), columns.begin(), column_converter);
			
			std::vector<size_t> value_indices(col_names.size() - 1);
			std::iota(value_indices.begin(), value_indices.end(), size_t(1));

			Eigen::MatrixXd values(EigenUtils::read_matrix(reader, value_indices, use_nans_for_missing));
			if (pad_nan_columns) {
				Padding::pad_nan_cols(values);
			}
			DataFrame<C, I, V> df(std::move(values), std::move(columns), std::move(index));
			//std::cerr << "Loaded:\n" << df;
			return df;
		}

		static DataFrame<C, I, V> from_csv_file(CSVFileReader& reader, bool use_nans_for_missing, bool pad_nan_columns) {
			C dflt_value;
			auto handler = CSVFileReader::return_default_value_handler(dflt_value);
			auto col_converter = CSVFileReader::default_converter<C, decltype(handler)>(handler);
			auto idx_converter = reader.default_converter_complaining<I>(INDEX_COLUMN_POSITION);
			return from_csv_file(reader, col_converter, idx_converter, use_nans_for_missing, pad_nan_columns);
		}

		static const size_type NOT_FOUND = std::numeric_limits<size_type>::max();	

		void to_csv(std::ostream& os, const CSV::Delimiter delimiter) const {
			// header
			os << "INDEX";
			for (size_type c = 0; c < nbr_cols_; ++c) {
				os << delimiter << columns_[c];
			}
			// index and values
			for (size_type r = 0; r < nbr_rows_; ++r) {
				os << "\n";
				os << index_[r];
				for (size_type c = 0; c < nbr_cols_; ++c) {
					os << delimiter << values_(r, c);
				}
			}
		}

		/** Append new column at the end.
		@throws std::domain_error If new_data has different number of rows than values()
		*/
		void append_column(const C& new_label, Eigen::Ref<const Eigen::VectorXd> new_data) {
			check_equals(nbr_rows_, new_data.size(), "DataFrame::append_column: size mismatch");
			columns_.push_back(new_label);
			values_.conservativeResize(Eigen::NoChange, nbr_cols_ + 1);
			values_.col(nbr_cols_) = new_data;
			++nbr_cols_;
		}
	private:
		values_type values_;
		columns_type columns_;
		index_type index_;		
		size_type nbr_rows_;
		size_type nbr_cols_;

		/** Return NOT_FOUND if no such label */
		template <class T> static size_type find_idx_unsafe(const std::vector<T>& labels, const T& label) {
			const auto it = std::find(labels.begin(), labels.end(), label);
			if (it != labels.end()) {
				const auto idx = static_cast<size_t>(std::distance(labels.begin(), it));
				assert(idx < NOT_FOUND);
				return idx;
			} else {
				return NOT_FOUND;
			}
		}

		template <class T> static size_type find_idx_safe(const std::vector<T>& labels, const T& label) {
			const auto it = std::find(labels.begin(), labels.end(), label);
			if (it != labels.end()) {
				return std::distance(labels.begin(), it);
			} else {
				throw std::out_of_range("DataFrame: no such label");
			}
		}

		template <class T> void sort_rows_by_kvpairs(std::vector<std::pair<T, size_type>>& kv, bool ascending = true) {
			auto comparator = [](const std::pair<T, size_type>& l, const std::pair<T, size_type>& r) { return l.first < r.first; };
			if (ascending) {
				std::sort(kv.begin(), kv.end(), comparator);
			} else {
				std::sort(kv.rbegin(), kv.rend(), comparator);
			}
			values_type new_values(nbr_rows(), nbr_cols());
			index_type new_index(nbr_rows());
			for (size_type i = 0; i < nbr_rows(); ++i) {
				const auto src_idx = kv[i].second;
				new_index[i] = index_[src_idx];
				new_values.row(i) = values_.row(src_idx);
			}
			values_.swap(new_values);
			index_.swap(new_index);
		}
	};	

	
	template <class C, class I, class V> std::ostream& operator<<(std::ostream& stream, const DataFrame<C, I, V>& df) {
		if (df.empty()) {
			stream << "[]\n";
		} else {
			df.to_csv(stream, CSV::Delimiter::TAB);
			stream << "\n";
		}
		return stream;
	}

	template <class C, class I, class V> void swap(DataFrame<C, I, V>& l, DataFrame<C, I, V>& r) {
		l.swap(r);
	}
}

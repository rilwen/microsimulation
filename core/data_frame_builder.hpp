#pragma once
#include "data_frame.hpp"
#include "preconditions.hpp"
#include <vector>

namespace averisera {
	/** DataFrame builder: col-wise or row-wise. Preserves rectangular shape of the DataFrame. */
	template <class C, class I, class V = double> class DataFrameBuilder {
	public:
		static DataFrameBuilder<C, I, V> make_rowwise(const std::vector<C>& columns, size_t expected_rows = 0) {
			DataFrameBuilder<C, I, V> builder(true);
			builder.columns_ = columns;
			builder.index_.reserve(expected_rows);
			builder.values_.reserve(expected_rows);
			return builder;
		}

		static DataFrameBuilder<C, I, V> make_colwise(const std::vector<I>& index, size_t expected_cols = 0) {
			DataFrameBuilder<C, I, V> builder(false);
			builder.index_ = index;
			builder.columns_.reserve(expected_cols);
			builder.values_.reserve(expected_cols);
			return builder;
		}

		DataFrame<C, I, V> build() const {
			const size_t nr = nbr_rows();
			const size_t nc = nbr_cols();
			typename DataFrame<C, I, V>::values_type nv(nr, nc);
			if (rowwise_) {
				for (size_t i = 0; i < nr; ++i) {
					for (size_t j = 0; j < nc; ++j) {
						nv(i, j) = values_[i][j];
					}
				}
			} else {
				for (size_t i = 0; i < nc; ++i) {
					for (size_t j = 0; j < nr; ++j) {
						nv(j, i) = values_[i][j];
					}
				}
			}
			std::vector<C> cols(columns_);
			std::vector<I> index(index_);
			return DataFrame<C, I, V>(std::move(nv), std::move(cols), std::move(index));
		}

		DataFrameBuilder<C, I, V>& set_columns(const std::vector<C>& columns) {
			check_that(rowwise_, "DataFrameBuilder: set_columns requires a rowwise builder");
			if (!empty()) {
				check_equals(nbr_cols(), columns.size(), "DataFrameBuilder: number of columns must be preserved");
			}
			columns_ = columns;
			return *this;
		}

		DataFrameBuilder<C, I, V>& add_row(I label , const std::vector<V>& new_row) {
			check_that(rowwise_, "DataFrameBuilder: add_row requires a rowwise builder");
			check_equals(nbr_cols(), new_row.size(), "DataFrameBuilder: row length must be consistent");
			index_.push_back(label);
			values_.push_back(new_row);
			return *this;
		}

		DataFrameBuilder<C, I, V>& add_col(C label, const std::vector<V>& new_col) {
			check_that(!rowwise_, "DataFrameBuilder: add_col requires a colwise builder");
			check_equals(nbr_rows(), new_col.size(), "DataFrameBuilder: column length must be consistent");
			columns_.push_back(label);
			values_.push_back(new_col);				
			return *this;
		}

		bool rowwise() const {
			return rowwise_;
		}

		bool colwise() const {
			return !rowwise();
		}

		bool empty() const {
			return values_.empty();
		}

		size_t nbr_rows() const {
			if (rowwise_) {
				return values_.size();
			} else {
				return index_.size();
			}
		}

		size_t nbr_cols() const {
			if (rowwise_) {
				return columns_.size();
			} else {
				return values_.size();
			}
		}
	private:
		bool rowwise_;
		std::vector<C> columns_;
		std::vector<I> index_;
		std::vector<std::vector<V>> values_;

		DataFrameBuilder(bool rowwise)
			: rowwise_(rowwise) {}
	};
}

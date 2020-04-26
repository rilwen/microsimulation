// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_EIGEN_H
#define __AVERISERA_EIGEN_H

#include "data_check_level.hpp"
#include "math_utils.hpp"
#include "running_mean.hpp"
#include <Eigen/Core>
#include <cassert>
#include <stdexcept>
#include <vector>

// Utilty functions helping to use Eigen++ effectively.

static_assert(EIGEN_VERSION_AT_LEAST(3, 3, 4), "You need to upgrade Eigen!");

namespace averisera {
	class CSVFileReader;

	namespace EigenUtils {
		/** A view of std::vector as Eigen::VectorXd */
		Eigen::Ref<Eigen::VectorXd> from_vec(std::vector<double>& vec);

		/** A view of std::vector as Eigen::VectorXd (const) */
		Eigen::Ref<const Eigen::VectorXd> from_vec(const std::vector<double>& vec);

		/** Vector from iterators. Does multiple passes over iterators. */
		template <class V, class I> Eigen::Matrix<V, Eigen::Dynamic, 1> vector_from_iters(I begin, const I end) {
			size_t i = std::distance(begin, end);
			return vector_from_iters(begin, end, i);
		}

		/** Vector from iterators given size.  */
		template <class V, class I> Eigen::Matrix<V, Eigen::Dynamic, 1> vector_from_iters(I begin, const I end, size_t size) {
			Eigen::Matrix<V, Eigen::Dynamic, 1> vec(size);
			std::copy(begin, end, vec.data());
			return vec;
		}

		/** Matrix from iterators given number of rows and columns. Iterators iterate over rows. Iterator value type should be iterable over. */
		template <class V, class I> Eigen::Matrix<V, Eigen::Dynamic, Eigen::Dynamic> matrix_from_iters(I begin, const I end, const size_t nrows, const size_t ncols) {
			Eigen::Matrix<V, Eigen::Dynamic, Eigen::Dynamic> mat(nrows, ncols);
			size_t i = 0;
			while (begin != end) {
				assert(i < nrows);
				size_t j = 0;
				for (const V& x : *begin) {
					assert(j < ncols);
					mat(i, j) = x;
					++j;
				}
				++begin;
				++i;
			}
			return mat;
		}

		/** Read selected columns as Eigen::MatrixXd */
		Eigen::MatrixXd read_matrix(CSVFileReader& reader, const std::vector<size_t>& indices, bool fill_with_nans);

		/** Read selected columns as Eigen::MatrixXd */
		Eigen::MatrixXd read_matrix(CSVFileReader& reader, const std::vector<std::string>& names, bool fill_with_nans);		

		/**
		Calculate column-wise means accurately.
		@param check_level Check level for input data
		@param indendent Whether columns are checked independently or together
		*/
		template <class V> Eigen::Matrix<V, Eigen::Dynamic, 1> accurate_mean_colwise(Eigen::Ref<const Eigen::Matrix<V, Eigen::Dynamic, Eigen::Dynamic>> matrix, DataCheckLevel check_level, bool independent) {
			Eigen::Matrix<V, Eigen::Dynamic, 1> v(matrix.cols());
			if (independent) {				
				for (decltype(matrix.cols()) c = 0; c < matrix.cols(); ++c) {
					RunningMean<V> m;
					auto it = matrix.col(c).data();
					const auto end = it + matrix.rows();
					switch (check_level) {
					case DataCheckLevel::ANY:
						for (; it != end; ++it) {
							m.add(*it);
						}
						break;
					case DataCheckLevel::NOT_NAN:
						for (; it != end; ++it) {
							m.add_if_not_nan(*it);
						}
						break;
					case DataCheckLevel::FINITE:
						for (; it != end; ++it) {
							m.add_if_finite(*it);
						}
						break;
					default:
						throw std::logic_error("EigenUtils: unknown data check level");
					}
					v[c] = m.mean();
				}				
			} else {
				std::vector<RunningMean<V>> rm(matrix.cols());
				for (decltype(matrix.rows()) r = 0; r < matrix.rows(); ++r) {
					const auto row(matrix.row(r));
					if (MathUtils::check_data(row, check_level)) {
						for (decltype(matrix.cols()) c = 0; c < matrix.cols(); ++c) {
							rm[c].add(row[c]);
						}
					}
				}
				for (decltype(matrix.cols()) c = 0; c < matrix.cols(); ++c) {
					v[c] = rm[c].mean();
				}
			}
			return v;
		}		

		/** Convert a matrix to a vector of columns. Useful for saving the matrix in a TAB file. */
		std::vector<std::vector<double>> as_vector_of_cols(const Eigen::MatrixXd& m);
	}
}

#endif // __AVERISERA_EIGEN_H

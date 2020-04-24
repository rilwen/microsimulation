/*
(C) Averisera Ltd 2014
*/
#include "object_array_2d.hpp"
#include "array_2d.hpp"
#include "object_value.hpp"

namespace averisera {
	namespace ObjArr2D {
		static void paste_matrix(const Eigen::MatrixXd& matrix, const size_t row_offset, Array2D<ObjectValue>& arr) {
			for (Eigen::MatrixXd::Index r = 0; r < matrix.rows(); ++r) {
				auto& row = arr[r + row_offset];
				for (Eigen::MatrixXd::Index c = 0; c < matrix.cols(); ++c) {
					row[c] = ObjectValue(matrix(r, c));
				}
			}
		}

		Array2D<ObjectValue> from(const Eigen::MatrixXd& matrix) {
			Array2D<ObjectValue> arr(matrix.rows(), matrix.cols());
			paste_matrix(matrix, 0, arr);
			return arr;
		}

		Array2D<ObjectValue> from(bool column, const Eigen::VectorXd& vector) {
			if (column) {
				return from(vector);
			}
			else {
				return from(vector.transpose());
			}
		}

		ObjectArray2D from(const Eigen::MatrixXd& matrix, const char* header) {
			Array2D<ObjectValue> arr(matrix.rows() + 1, matrix.cols());
			arr(0, 0) = header;
			arr[0].resize(1);
			paste_matrix(matrix, 1, arr);
			return arr;
		}

		ObjectArray2D from(bool column, const Eigen::VectorXd& vector, const char* header) {
			if (column) {
				return from(vector, header);
			} else {
				return from(vector.transpose(), header);
			}
		}

		ObjectArray2D from(double value, const char* header) {
			Array2D<ObjectValue> arr(2, 1);
			arr(0, 0) = header;
			arr(1, 0) = value;
			return arr;
		}		
	}
}
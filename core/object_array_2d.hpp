/*
(C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_OBJECT_ARRAY_2D_H
#define __AVERISERA_OBJECT_ARRAY_2D_H

#include <Eigen/Core>
#include <vector>
#include "array_2d.hpp"

namespace averisera {

	class ObjectValue;

	typedef Array2D < ObjectValue > ObjectArray2D;

	/// 2D array of generic objects.
	namespace ObjArr2D {
		/// Create a 2D array from a matrix.
		/// @param matrix Input matrix.
		/// @return 2D object array.
		ObjectArray2D from(const Eigen::MatrixXd& matrix);

		/// Create a 2D array from a vector.
		/// @param column Is the vector a column vector.
		/// @param vector Input vector.
		/// @return 2D object array.
		ObjectArray2D from(bool column, const Eigen::VectorXd& vector);

		/// Create a 2D array with a header from a matrix.
		/// @param matrix Input matrix.
		/// \header Header text.
		/// @return 2D object array.
		ObjectArray2D from(const Eigen::MatrixXd& matrix, const char* header);

		/// Create a 2D array with a header from a vector.
		/// @param column Is the vector a column vector.
		/// @param vector Input vector.
		/// \header Header text.
		/// @return 2D object array.
		ObjectArray2D from(bool column, const Eigen::VectorXd& vector, const char* header);

		/// Create a 2D array with a header from a scalar.
		/// @param value Input scalar.
		/// \header Header text.
		/// @return 2D object array.
		ObjectArray2D from(double value, const char* header);

		/// Create a 2D array with a header from a vector.
		/// @param column Is the vector a column vector.
		/// @param vector Input vector.
		/// \header Header text.
		/// @return 2D object array.
		template <class V> ObjectArray2D from(bool column, const std::vector<V>& vector, const char* header) {
			const size_t v_len = vector.size();
			if (column) {
				Array2D<ObjectValue> arr(v_len + 1, 1);
				arr(0, 0) = header;
				for (size_t i = 0; i < v_len; ++i) {
					arr(i + 1, 0) = vector[i];
				}
				return arr;
			} else {
				Array2D<ObjectValue> arr;
				arr(0, 0) = header;
				arr.ensure_region(1, 0, 2, v_len);
				std::copy(vector.begin(), vector.end(), arr[1].begin());
				return arr;
			}
		}
	}


}

#endif

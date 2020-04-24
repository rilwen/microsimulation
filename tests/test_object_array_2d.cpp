/*
(C) Averisera Ltd 2014
*/
#include <gtest/gtest.h>
#include "core/object_array_2d.hpp"
#include "core/object_value.hpp"
#include "core/array_2d.hpp"

using namespace averisera;

void cmp_array_matrix(const char* name, const Array2D<ObjectValue>& arr, const Eigen::MatrixXd& m, const unsigned int row_offset) {
	ASSERT_EQ(static_cast<size_t>(m.rows() + row_offset), arr.size());
	for (Eigen::MatrixXd::Index r = 0; r < m.rows(); ++r) {
		const auto& row = arr[r + row_offset];
		ASSERT_EQ(static_cast<size_t>(m.cols()), row.size()) << name << ": " << r;
		for (Eigen::MatrixXd::Index c = 0; c < m.cols(); ++c) {
			ASSERT_EQ(m(r, c), row[c].as_double()) << name << ": " << r << " " << c;
		}
	}
}

TEST(ObjectArray2D, Test) {
	Eigen::MatrixXd m(3, 2);
	m << 1, 2,
		3, 4,
		5, 6;
	Eigen::VectorXd v(2);
	v << 10, 20;
	Array2D<ObjectValue> arr = ObjArr2D::from(m);
	cmp_array_matrix("Matrix No Header", arr, m, 0);
	arr = ObjArr2D::from(m, "Header");	
	cmp_array_matrix("Matrix With Header", arr, m, 1);
	ASSERT_EQ(1u, arr[0].size());
	ASSERT_EQ("Header", arr[0][0].as_string());
	arr = ObjArr2D::from(true, v);
	cmp_array_matrix("Vector No Header", arr, v, 0);
	arr = ObjArr2D::from(false, v);
	cmp_array_matrix("RowVector No Header", arr, v.transpose(), 0);

	arr = ObjArr2D::from(true, v, "Header");
	cmp_array_matrix("Vector With Header", arr, v, 1);
	ASSERT_EQ(1u, arr[0].size());
	ASSERT_EQ("Header", arr[0][0].as_string());
	arr = ObjArr2D::from(false, v, "Header");
	cmp_array_matrix("RowVector With Header", arr, v.transpose(), 1);
	ASSERT_EQ(1u, arr[0].size());
	ASSERT_EQ("Header", arr[0][0].as_string());

	arr = ObjArr2D::from(0.1, "Value");
	ASSERT_EQ(2u, arr.size());
	ASSERT_EQ(1u, arr[0].size());
	ASSERT_EQ("Value", arr[0][0].as_string());
	ASSERT_EQ(1u, arr[1].size());
	ASSERT_EQ(0.1, arr[1][0].as_double());

	const std::vector<double> vec = { 0.1, 0.2 };
	arr = ObjArr2D::from(true, vec, "Vector");
	ASSERT_EQ(1 + vec.size(), arr.size());
	ASSERT_EQ(1u, arr[0].size());
	ASSERT_EQ("Vector", arr[0][0].as_string());
	for (size_t i = 0; i < vec.size(); ++i) {
		ASSERT_EQ(1u, arr[i + 1].size()) << i;
		ASSERT_EQ(vec[i], arr[i + 1][0].as_double()) << i;
	}
	arr = ObjArr2D::from(false, vec, "Vector");
	ASSERT_EQ(2u, arr.size());
	ASSERT_EQ(1u, arr[0].size());
	ASSERT_EQ("Vector", arr[0][0].as_string());
	ASSERT_EQ(vec.size(), arr[1].size());
	for (size_t i = 0; i < vec.size(); ++i) {
		ASSERT_EQ(vec[i], arr[1][i].as_double()) << i;
	}
}

/*
(C) Averisera Ltd 2014
*/
#include <gtest/gtest.h>
#include "core/array_2d.hpp"

TEST(Array2D, Constructor) {
	averisera::Array2D<int> v(2);
	ASSERT_EQ(2u, v.size());
	for (auto& r : v) {
		ASSERT_EQ(0u, r.size());
	}
	averisera::Array2D<int> v2(3, 2);
	ASSERT_EQ(3u, v2.size());
	for (auto& r : v2) {
		ASSERT_EQ(2u, r.size());
	}

	averisera::Array2D<double> d2(averisera::Array2D<double>::from(v2));
	ASSERT_EQ(3u, d2.size());
	for (auto& r : d2) {
		ASSERT_EQ(2u, r.size());
	}
}

TEST(Array2D, MaxRowSize) {
	averisera::Array2D<int> v2(3, 2);
	ASSERT_EQ(2u, v2.max_row_size());
	v2.ensure_elem(2, 4);
	ASSERT_EQ(5u, v2.max_row_size());
}

TEST(Array2D, Paste) {
	averisera::Array2D<double> src;
	src.ensure_region(0, 0, 2, 2);
	src[0][0] = 0; src[0][1] = 1;
	src[1][0] = -1; src[1][1] = 0;
	averisera::Array2D<double> dst;
	dst.paste(src, 1, 1);
	ASSERT_EQ(3u, dst.size());
	for (size_t i = 0; i < 2; ++i) {
		ASSERT_EQ(3u, dst[i + 1].size());
		for (size_t j = 0; j < 2; ++j) {
			ASSERT_EQ(src[i][j], dst[i + 1][j + 1]);
		}
	}
}

TEST(Array2D, Access) {
	averisera::Array2D<double> arr;
	arr(0, 2) = 0.1;
	ASSERT_EQ(1u, arr.size());
	ASSERT_EQ(3u, arr[0].size());
	ASSERT_EQ(0.1, arr[0][2]);
	arr(1, 0) = 0.2;
	ASSERT_EQ(2u, arr.size());
	ASSERT_EQ(1u, arr[1].size());
	ASSERT_EQ(0.2, arr[1][0]);
}

TEST(Array2D, Ensure) {
	averisera::Array2D<double> v;
	v.ensure_row(3);
	ASSERT_EQ(4u, v.size());
	v.ensure_elem(0, 1);
	ASSERT_EQ(2u, v[0].size());
	v.ensure_region(1, 2, 5, 4);
	ASSERT_EQ(5u, v.size());
	for (size_t r = 1; r < 5; ++r) {
		ASSERT_EQ(4u, v[r].size()) << r;
	}
}

TEST(Array2D, MoveConstructor) {
    averisera::Array2D<double> orig(2, 2);
    const double* p = &orig[0][0];
    averisera::Array2D<double> copy(std::move(orig));
    ASSERT_EQ(p, &copy[0][0]);
    ASSERT_EQ(0u, orig.size());
}


TEST(Array2D, Swap) {
    averisera::Array2D<double> orig(2, 2);
    const double* p = &orig[0][0];
    averisera::Array2D<double> copy;
    copy.swap(orig);
    ASSERT_EQ(p, &copy[0][0]);
    ASSERT_EQ(0u, orig.size());
}


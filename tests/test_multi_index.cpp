#include <gtest/gtest.h>
#include "core/multi_index.hpp"
#include <Eigen/Core>

TEST(MultiIndex, ConstructorEqualSizes) {
	averisera::MultiIndex mi(2, 4);
	ASSERT_EQ(16u, mi.flat_size());
	ASSERT_EQ(0u, mi.flat_index());
	ASSERT_EQ(2u, mi.dim());
	ASSERT_EQ(2u, mi.indices().size());
	ASSERT_EQ(0u, mi.indices()[0]);
	ASSERT_EQ(0u, mi.indices()[1]);
}

//TEST(MultiIndex, ConstructorSizes) {
//	std::vector<size_t> sizes = { 3, 4 };
//	averisera::MultiIndex mi(sizes);
//	ASSERT_EQ(12, mi.flat_size());
//	ASSERT_EQ(0, mi.flat_index());
//	ASSERT_EQ(2, mi.dim());
//	ASSERT_EQ(2, mi.indices().size());
//	ASSERT_EQ(0, mi.indices()[0]);
//	ASSERT_EQ(0, mi.indices()[1]);
//}

TEST(MultiIndex, Iteration) {
	const size_t dim = 2;
	const size_t size = 3;
	averisera::MultiIndex mi(dim, size);
	Eigen::MatrixXd actual(9, dim);
	unsigned int r = 0;
	std::vector<size_t> decomposed(dim);
	while (mi.flat_index() < mi.flat_size()) {
		ASSERT_EQ(r, mi.flat_index()) << r;
		for (unsigned int i = 0; i < mi.dim(); ++i) {			
			actual(mi.flat_index(), i) = static_cast<double>(mi.indices()[i]);			
		}
		averisera::MultiIndex::decompose(mi.flat_index(), size, dim, decomposed);
		for (size_t i = 0; i < dim; ++i) {
			ASSERT_EQ(mi.indices()[i], decomposed[i]) << mi.flat_index() << " " << i;
		}
		ASSERT_EQ(mi.flat_index(), averisera::MultiIndex::flatten(size, decomposed)) << mi.flat_index();
		++mi;
		++r;
	}
	for (unsigned int i = 0; i < mi.dim(); ++i) {
		ASSERT_EQ(0u, mi.indices()[i]);
	}
	Eigen::MatrixXd expected(9, dim);
	expected << 0, 0,
		1, 0,
		2, 0,
		0, 1,
		1, 1,
		2, 1,
		0, 2,
		1, 2,
		2, 2;
	for (r = 0; r < 9; ++r) {
		for (size_t c = 0; c < 2; ++c) {
			ASSERT_EQ(expected(r, c), actual(r, c)) << r << " " << c;
		}
	}
}

TEST(MultiIndex, Reset) {
	averisera::MultiIndex mi(2, 4);
	++mi;
	ASSERT_EQ(1u, mi.flat_index());
	mi.reset();
	ASSERT_EQ(0u, mi.flat_index());
	for (unsigned int i = 0; i < mi.dim(); ++i) {
		ASSERT_EQ(0u, mi.indices()[i]);
	}
}

TEST(MultiIndex, Index) {
	averisera::MultiIndex mi(2, 4);
	++mi.index(0);
	ASSERT_EQ(1u, mi.flat_index());
	ASSERT_EQ(std::vector<size_t>({ 1, 0 }), mi.indices());
	++mi.index(1);
	ASSERT_EQ(5u, mi.flat_index());
	ASSERT_EQ(std::vector<size_t>({ 1, 1 }), mi.indices());
	mi.index(0) = 0;
	ASSERT_EQ(4u, mi.flat_index());
	ASSERT_EQ(std::vector<size_t>({ 0, 1 }), mi.indices());
	mi.index(1) = 0;
	ASSERT_EQ(0u, mi.flat_index());
	ASSERT_EQ(std::vector<size_t>({ 0, 0 }), mi.indices());

	mi = 5;
	ASSERT_EQ(5u, mi.flat_index());
	ASSERT_EQ(std::vector<size_t>({ 1, 1 }), mi.indices());
}

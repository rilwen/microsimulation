/*
(C) Averisera Ltd 2014
*/
#include <gtest/gtest.h>
#include "core/index_iterator.hpp"
#include <Eigen/Core>

TEST(IndexIterator, Test) {
	Eigen::VectorXd vec(3);
	vec << 0, 1, 2;
	auto it = averisera::make_index_iterator(vec, 1);
	ASSERT_EQ(1, *it);
	*it = 0.2;
	ASSERT_EQ(0.2, *it);
	*it = 1;
	++it;
	ASSERT_EQ(2, *it);
	--it;
	ASSERT_EQ(1, *it);
	const auto it2 = it;
	*it2 = 0.2;
	ASSERT_EQ(0.2, *it);
	*it = 1;
	ASSERT_EQ(1, *it2);
	ASSERT_EQ(it, it2);
	ASSERT_EQ(1u, it.pos());
	++it;
	ASSERT_NE(it, it2);
	ASSERT_EQ(1, it - it2);
	ASSERT_EQ(-1, it2 - it);
	it = it2 - 1;
	ASSERT_EQ(0u, it.pos());
	it += 1;
	ASSERT_EQ(1u, it.pos());
	it -= 1;
	ASSERT_EQ(0u, it.pos());
	it = it2 + 1;
	ASSERT_EQ(2u, it.pos());
	it = averisera::make_index_iterator_begin(vec);
	ASSERT_EQ(0u, it.pos());
	it = averisera::make_index_iterator_end(vec);
	ASSERT_EQ(3u, it.pos());
	const Eigen::VectorXd v2(vec);
	const auto vend = averisera::make_index_iterator_end(v2);
	it = averisera::make_index_iterator_begin(vec);
	for (auto cit = averisera::make_index_iterator_begin(v2); cit != vend; ++cit) {
		ASSERT_EQ(*cit, *it) << cit.pos();
		++it;
	}
}

TEST(IndexIterator, Default) {
    averisera::IndexIterator<Eigen::VectorXd> it;
    ASSERT_FALSE(it);
}

TEST(IndexIterator, PreDecIncrement) {
	std::vector<double> v({ 1,2,3 });
	auto it1 = averisera::make_index_iterator_begin(v);
	auto it2 = ++it1;
	ASSERT_EQ(2.0, *it1);
	ASSERT_EQ(it1, it2);
	it2 = --it1;
	ASSERT_EQ(1.0, *it1);
	ASSERT_EQ(it1, it2);
}

TEST(IndexIterator, PostDecIncrement) {
	std::vector<double> v({ 1,2,3 });
	auto it1 = averisera::make_index_iterator_begin(v);
	auto it2 = it1++;
	ASSERT_EQ(2.0, *it1);
	ASSERT_EQ(it1, it2 + 1);
	it2 = it1--;
	ASSERT_EQ(1.0, *it1);
	ASSERT_EQ(it2, it1 + 1);
}
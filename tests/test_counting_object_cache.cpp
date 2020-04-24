/*
* (C) Averisera Ltd 2019
*/
#include <gtest/gtest.h>
#include "core/counting_object_cache.hpp"

using namespace averisera;

TEST(CountingObjectCache, test) {
	CountingObjectCache<double> cache;
	ASSERT_EQ(1, cache.store("foo", std::make_shared<double>(0.1)));
	ASSERT_EQ(2, cache.store("foo", std::make_shared<double>(0.1)));
	ASSERT_EQ(3, cache.store("foo", std::make_shared<double>(0.2)));
	ASSERT_EQ(1, cache.store("bar", std::make_shared<double>(0.3)));
	ASSERT_EQ(2, cache.store("bar", std::make_shared<double>(0.3)));
	cache.sweep();
	ASSERT_EQ(1, cache.store("foo", std::make_shared<double>(0.1)));
}
// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/object_cache.hpp"

using namespace averisera;

TEST(ObjectCache, constructor) {
	ObjectCache<int> cache;
	ASSERT_EQ(0, cache.size());
	ASSERT_TRUE(cache.empty());
	ASSERT_EQ(0, cache.number_used());
}

TEST(ObjectCache, store) {
	ObjectCache<double> cache;
	const std::string tag("numer");
	{
		std::shared_ptr<double> obj = std::make_shared<double>(0.44);
		cache.store(tag, obj);
		ASSERT_EQ(1, cache.size());
		ASSERT_FALSE(cache.empty());
		ASSERT_EQ(1, cache.number_used()) << "Stored object used by a local shared_ptr";
		ASSERT_TRUE(cache.contains(tag));
	}
	ASSERT_TRUE(cache.contains(tag));
	ASSERT_EQ(1, cache.size());
	ASSERT_FALSE(cache.empty());
	ASSERT_EQ(0, cache.number_used()) << "Stored object not used anywhere because the local shared_ptr went out of scope";
}

TEST(ObjectCache, retrieve) {
	ObjectCache<double> cache;
	double val = 0.51;
	const std::string tag("number");
	cache.store(tag, std::make_shared<double>(val));
	ASSERT_EQ(1, cache.size());
	ASSERT_FALSE(cache.empty());
	ASSERT_EQ(0, cache.number_used());
	auto ptr = cache.retrieve(tag);
	ASSERT_EQ(val, *ptr);
	ASSERT_EQ(1, cache.number_used());
	auto ptr2 = cache.retrieve(tag);
	ASSERT_EQ(ptr.get(), ptr2.get());
}

TEST(ObjectCache, retrieve_throws) {
	ObjectCache<double> cache;
	ASSERT_THROW(cache.retrieve("goo"), std::invalid_argument);
}

TEST(ObjectCache, sweep) {
	ObjectCache<double> cache;
	cache.store("a", std::make_shared<double>(0.11));
	auto b = std::make_shared<double>(0.55);
	cache.store("b", b);
	ASSERT_TRUE(cache.contains("a"));
	ASSERT_TRUE(cache.contains("b"));
	ASSERT_EQ(2, cache.size());
	ASSERT_EQ(1, cache.sweep());
	ASSERT_FALSE(cache.contains("a"));
	ASSERT_TRUE(cache.contains("b"));
	ASSERT_EQ(1, cache.size());
	ASSERT_EQ(0, cache.sweep());
}

TEST(ObjectCache, contains) {
	ObjectCache<double> cache;
	ASSERT_FALSE(cache.contains("foo"));
	cache.store("foo", std::make_shared<double>(0.11));
	ASSERT_TRUE(cache.contains("foo"));
}
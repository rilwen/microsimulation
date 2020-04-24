#include <gtest/gtest.h>
#include "core/inclusion.hpp"
#include "core/range.hpp"

using namespace averisera;

TEST(Inclusion, InclusionVectorsInt) {
	std::vector<Range<int>> v1({ Range<int>(0, 1), Range<int>(2, 3), Range<int>(4, 5) });
	std::vector<Range<int>> v2({ Range<int>(0, 1), Range<int>(2, 5) });
	ASSERT_EQ(InclusionRelation::EQUALS, Inclusion::disjoint_elements_inclusion(v1, v1));
	ASSERT_EQ(InclusionRelation::CONTAINS, Inclusion::disjoint_elements_inclusion(v2, v1));
	ASSERT_EQ(InclusionRelation::IS_CONTAINED_BY, Inclusion::disjoint_elements_inclusion(v1, v2));
	std::vector<Range<int>> v3({ Range<int>(0, 1), Range<int>(4, 5) });
	ASSERT_EQ(InclusionRelation::CONTAINS, Inclusion::disjoint_elements_inclusion(v1, v3));
	ASSERT_EQ(InclusionRelation::IS_CONTAINED_BY, Inclusion::disjoint_elements_inclusion(v3, v1));
	std::vector<Range<int>> v4({ Range<int>(0, 2), Range<int>(4, 5) });
	ASSERT_EQ(InclusionRelation::UNDEFINED, Inclusion::disjoint_elements_inclusion(v1, v4));
	ASSERT_EQ(InclusionRelation::UNDEFINED, Inclusion::disjoint_elements_inclusion(v4, v1));
	ASSERT_EQ(InclusionRelation::UNDEFINED, Inclusion::disjoint_elements_inclusion(v2, v4));
	ASSERT_EQ(InclusionRelation::UNDEFINED, Inclusion::disjoint_elements_inclusion(v4, v2));
	ASSERT_EQ(InclusionRelation::CONTAINS, Inclusion::disjoint_elements_inclusion(v4, v3));
	ASSERT_EQ(InclusionRelation::IS_CONTAINED_BY, Inclusion::disjoint_elements_inclusion(v3, v4));
}

TEST(Inclusion, InclusionVectorsUChar) {
    const unsigned char a = 'A';
	const unsigned char b = 'B';
	const unsigned char c = 'C';
    typedef Range<unsigned char> rng;
    typedef std::vector<rng> vec;
    vec v1({rng(a, b), rng(c, c)});
    vec v2({rng(a, a), rng(b, b), rng(c, c)});
    ASSERT_EQ(InclusionRelation::CONTAINS, Inclusion::disjoint_elements_inclusion(v1, v2));
}

TEST(Inclusion, InclusionVectorsUSetUChar) {
    const unsigned char a = 'A';
	const unsigned char b = 'B';
	const unsigned char c = 'C';
    typedef std::unordered_set<unsigned char> set;
    typedef std::vector<set> vec;
    vec v1({set({a, b}), set({c})});
    vec v2({set({a}), set({b}), set({c})});
    ASSERT_EQ(InclusionRelation::CONTAINS, Inclusion::disjoint_elements_inclusion(v1, v2));
}

TEST(Inclusion, USetUChar) {
    const unsigned char a = 'A';
	const unsigned char b = 'B';
	//const unsigned char c = 'C';
    typedef std::unordered_set<unsigned char> set;
    ASSERT_TRUE(Inclusion::contains(set({a, b}), set({a})));
    ASSERT_TRUE(Inclusion::is_contained_by(set({a}), set({a, b})));
}

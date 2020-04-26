// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-core/person_attributes.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(PersonAttributes, Size) {
    PersonAttributes pa(Sex::MALE, 0);
    ASSERT_EQ(2u, sizeof(pa));
}

TEST(PersonAttributes, Constructor) {
    PersonAttributes he(Sex::MALE, 2);
    ASSERT_EQ(Sex::MALE, he.sex());
    ASSERT_EQ(2u, he.ethnicity());
    PersonAttributes she(Sex::FEMALE, 1);
    ASSERT_EQ(Sex::FEMALE, she.sex());
    ASSERT_EQ(1u, she.ethnicity());
}

TEST(PersonAttributes, Eq) {
    PersonAttributes a1(Sex::MALE, 0);
    PersonAttributes a2(Sex::FEMALE, 0);
    PersonAttributes a3(Sex::MALE, 1);
    PersonAttributes a4(Sex::MALE, 0);
    ASSERT_EQ(a1, a1);
    ASSERT_EQ(a1, a4);
    ASSERT_FALSE(a1 == a2);
    ASSERT_FALSE(a1 == a3);
}

TEST(PersonAttributes, Swap) {
    PersonAttributes a1(Sex::MALE, 0);
    PersonAttributes a2(Sex::FEMALE, 1);
    std::swap(a1, a2);
    ASSERT_EQ(Sex::FEMALE, a1.sex());
    ASSERT_EQ(Sex::MALE, a2.sex());
    ASSERT_EQ(1, a1.ethnicity());
    ASSERT_EQ(0, a2.ethnicity());
}

TEST(PersonAttributes, Hash) {
	PersonAttributes a1(Sex::MALE, 0);
	PersonAttributes a2(Sex::FEMALE, 0);
	PersonAttributes a3(Sex::MALE, 1);
	PersonAttributes a4(Sex::FEMALE, 1);
	size_t h1 = std::hash<PersonAttributes>()(a1);
	size_t h2 = std::hash<PersonAttributes>()(a2);
	size_t h3 = std::hash<PersonAttributes>()(a3);
	size_t h4 = std::hash<PersonAttributes>()(a4);
	ASSERT_NE(h1, h2);
	ASSERT_NE(h1, h3);
	ASSERT_NE(h1, h4);
	ASSERT_NE(h2, h3);
	ASSERT_NE(h2, h4);
	ASSERT_NE(h3, h4);
}

TEST(PersonAttributes, Less) {
	const PersonAttributes f0(Sex::FEMALE, 0);
	const PersonAttributes f1(Sex::FEMALE, 1);
	const PersonAttributes m0(Sex::MALE, 0);
	const PersonAttributes m1(Sex::MALE, 1);
	ASSERT_TRUE(f0 < f1);
	ASSERT_TRUE(f0 < m0);
	ASSERT_TRUE(f0 < m1);
	ASSERT_TRUE(m0 < m1);
	ASSERT_FALSE(f0 < f0);
	ASSERT_FALSE(m0 < m0);
	ASSERT_FALSE(f1 < f0);
	ASSERT_FALSE(m0 < f0);
	ASSERT_FALSE(m1 < m0);	
}

TEST(PersonAttributes, LessOrEqual) {
	const PersonAttributes f0(Sex::FEMALE, 0);
	const PersonAttributes f1(Sex::FEMALE, 1);
	const PersonAttributes m0(Sex::MALE, 0);
	const PersonAttributes m1(Sex::MALE, 1);
	ASSERT_TRUE(f0 <= f1);
	ASSERT_TRUE(f0 <= m0);
	ASSERT_TRUE(f0 <= m1);
	ASSERT_TRUE(m0 <= m1);
	ASSERT_TRUE(f0 <= f0);
	ASSERT_TRUE(m0 <= m0);
	ASSERT_FALSE(f1 <= f0);
	ASSERT_FALSE(m0 <= f0);
	ASSERT_FALSE(m1 <= m0);
}

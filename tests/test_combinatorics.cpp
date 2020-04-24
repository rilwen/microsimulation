/*
(C) Averisera Ltd 2017
*/
#include <gtest/gtest.h>
#include "core/combinatorics.hpp"

using namespace averisera;

TEST(Combinatorics, partial_partition) {
	ASSERT_EQ(3, Combinatorics::partial_partition(3, 6));
	ASSERT_EQ(0, Combinatorics::partial_partition(6, 3));
	ASSERT_EQ(1, Combinatorics::partial_partition(0, 0));
	ASSERT_EQ(0, Combinatorics::partial_partition(0, 4));
	ASSERT_EQ(0, Combinatorics::partial_partition(4, 0));
	ASSERT_EQ(1, Combinatorics::partial_partition(1, 1019));
	ASSERT_EQ(8, Combinatorics::partial_partition(2, 16));
	ASSERT_EQ(128, Combinatorics::partial_partition(2, 256));
}

TEST(Combinatorics, partial_partition_restricted_size) {
	ASSERT_EQ(1, Combinatorics::partial_partition_restricted_size(3, 6, 2));
	ASSERT_EQ(3, Combinatorics::partial_partition_restricted_size(3, 6, 0));
	ASSERT_EQ(3, Combinatorics::partial_partition_restricted_size(3, 6, 1));
	ASSERT_EQ(0, Combinatorics::partial_partition_restricted_size(3, 5, 2));
	ASSERT_EQ(1, Combinatorics::partial_partition_restricted_size(2, 5, 2));
	ASSERT_EQ(0, Combinatorics::partial_partition_restricted_size(5, 5, 3));
	ASSERT_EQ(1, Combinatorics::partial_partition_restricted_size(1, 1019, 2));
}

TEST(Combinatorics, partial_composition) {
	ASSERT_EQ(Combinatorics::partial_composition(2, 5), 4);
	ASSERT_EQ(Combinatorics::partial_composition(3, 5), 6);
	ASSERT_EQ(Combinatorics::partial_composition(1, 5), 1);
	ASSERT_EQ(Combinatorics::partial_composition(5, 5), 1);
	ASSERT_EQ(Combinatorics::partial_composition(0, 0), 1);
	ASSERT_EQ(Combinatorics::partial_composition(0, 4), 0);
	ASSERT_EQ(Combinatorics::partial_composition(4, 0), 0);
}

TEST(Combinatorics, partial_composition_restricted_size) {
	ASSERT_EQ(Combinatorics::partial_composition_restricted_size(2, 5, 1), 4);
	ASSERT_EQ(Combinatorics::partial_composition_restricted_size(2, 5, 0), 4);
	ASSERT_EQ(Combinatorics::partial_composition_restricted_size(2, 5, 2), 2);
	ASSERT_EQ(Combinatorics::partial_composition_restricted_size(3, 5, 2), 0);
	ASSERT_EQ(Combinatorics::partial_composition_restricted_size(1, 5, 3), 1);
	ASSERT_EQ(Combinatorics::partial_composition_restricted_size(1, 5, 6), 0);
	ASSERT_EQ(Combinatorics::partial_composition_restricted_size(5, 5, 1), 1);
	ASSERT_EQ(Combinatorics::partial_composition_restricted_size(5, 5, 3), 0);
	ASSERT_EQ(Combinatorics::partial_composition_restricted_size(0, 0, 0), 1);
	ASSERT_EQ(Combinatorics::partial_composition_restricted_size(0, 4, 1), 0);
	ASSERT_EQ(Combinatorics::partial_composition_restricted_size(4, 0, 1), 0);
}

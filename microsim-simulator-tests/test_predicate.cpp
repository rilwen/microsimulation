// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "mock_predicate.hpp"
#include "microsim-simulator/contexts.hpp"
#include "microsim-simulator/predicate.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(Predicate, Sum) {
    MockPredicate p1(1);
    MockPredicate p2(2);
    Contexts ctx;
    std::shared_ptr<const Predicate<int>> or1 = p1.sum(p2);
    ASSERT_TRUE(or1->select(1, ctx));
    ASSERT_TRUE(or1->select(2, ctx));
    ASSERT_FALSE(or1->select(0, ctx));
	ASSERT_TRUE(or1->select_alive(1, ctx));
	ASSERT_TRUE(or1->select_alive(2, ctx));
	ASSERT_FALSE(or1->select_alive(0, ctx));
    ASSERT_FALSE(or1->always_true());
    ASSERT_TRUE(or1->select_out_of_context(1));
    ASSERT_TRUE(or1->select_out_of_context(2));
    ASSERT_FALSE(or1->select_out_of_context(0));
    ASSERT_FALSE(or1->always_true_out_of_context());
    or1 = p1.sum(std::make_shared<MockPredicate>(2));
    ASSERT_TRUE(or1->select(1, ctx));
    ASSERT_TRUE(or1->select(2, ctx));
    ASSERT_FALSE(or1->select(0, ctx));
	ASSERT_TRUE(or1->select_alive(1, ctx));
	ASSERT_TRUE(or1->select_alive(2, ctx));
	ASSERT_FALSE(or1->select_alive(0, ctx));
    ASSERT_FALSE(or1->always_true());
    ASSERT_TRUE(or1->select_out_of_context(1));
    ASSERT_TRUE(or1->select_out_of_context(2));
    ASSERT_FALSE(or1->select_out_of_context(0));
    ASSERT_FALSE(or1->always_true_out_of_context());
}

TEST(Predicate, Product) {
    MockPredicate p1(1);
    MockPredicate p2(2);
    Contexts ctx;
    std::shared_ptr<const Predicate<int>> r1 = p1.product(p2);
    ASSERT_FALSE(r1->select(1, ctx));
    ASSERT_FALSE(r1->select(2, ctx));
    ASSERT_FALSE(r1->select(0, ctx));
	ASSERT_FALSE(r1->select_alive(1, ctx));
	ASSERT_FALSE(r1->select_alive(2, ctx));
	ASSERT_FALSE(r1->select_alive(0, ctx));
    ASSERT_FALSE(r1->always_true());
    ASSERT_FALSE(r1->select_out_of_context(1));
    ASSERT_FALSE(r1->select_out_of_context(2));
    ASSERT_FALSE(r1->select_out_of_context(0));
    ASSERT_FALSE(r1->always_true_out_of_context());
    r1 = p2.product(std::make_shared<MockPredicate>(2));
    ASSERT_TRUE(r1->select(2, ctx));
    ASSERT_FALSE(r1->select(1, ctx));
    ASSERT_FALSE(r1->select(0, ctx));
	ASSERT_TRUE(r1->select_alive(2, ctx));
	ASSERT_FALSE(r1->select_alive(1, ctx));
	ASSERT_FALSE(r1->select_alive(0, ctx));
    ASSERT_FALSE(r1->always_true());
    ASSERT_TRUE(r1->select_out_of_context(2));
    ASSERT_FALSE(r1->select_out_of_context(1));
    ASSERT_FALSE(r1->select_out_of_context(0));
    ASSERT_FALSE(r1->always_true_out_of_context());
}

TEST(Predicate, Not) {
    MockPredicate p1(1);
    Contexts ctx;
    std::shared_ptr<const Predicate<int>> r1 = p1.negate();
    ASSERT_FALSE(r1->select(1, ctx));
    ASSERT_TRUE(r1->select(2, ctx));
    ASSERT_FALSE(r1->always_true());
	ASSERT_FALSE(r1->select_alive(1, ctx));
	ASSERT_TRUE(r1->select_alive(2, ctx));
	ASSERT_FALSE(r1->select_out_of_context(1));
    ASSERT_TRUE(r1->select_out_of_context(2));
    ASSERT_FALSE(r1->always_true_out_of_context());
}

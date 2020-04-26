// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/predicate/pred_or.hpp"
#include "microsim-simulator/contexts.hpp"
#include "mock_predicate.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(PredOr, Sum) {
    Contexts ctx;
    const auto m1 = std::make_shared<MockPredicate>(1);
    const auto m2 = std::make_shared<MockPredicate>(2);
    const PredOr<int> pred_or({m1});
    const auto r1 = pred_or.sum(m2);
	std::stringstream ss;
	r1->print(ss);
	ASSERT_EQ("Or(Mock(1), Mock(2))", ss.str());
    const auto m3 = std::make_shared<MockPredicate>(3);
    const auto r2 = r1->sum(m3);
    const std::shared_ptr<const PredOr<int>> po2 = std::dynamic_pointer_cast<const PredOr<int>>(r2);
    ASSERT_EQ(3u, po2->size());
    ASSERT_TRUE(po2->select(1, ctx));
    ASSERT_TRUE(po2->select(2, ctx));
    ASSERT_TRUE(po2->select(3, ctx));
    ASSERT_TRUE(po2->select_out_of_context(1));
    ASSERT_TRUE(po2->select_out_of_context(2));
    ASSERT_TRUE(po2->select_out_of_context(3));

	const auto minact = std::make_shared<MockPredicate>(1, false);
	const auto minact2 = std::make_shared<MockPredicate>(1, false);
	const PredOr<int> pred_or2({ minact, minact2 });
	ASSERT_FALSE(pred_or2.active(Date(1989, 6, 4)));
}

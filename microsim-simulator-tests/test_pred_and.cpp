#include <gtest/gtest.h>
#include "microsim-simulator/predicate/pred_and.hpp"
#include "microsim-simulator/contexts.hpp"
#include "mock_predicate.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(PredAnd, Product) {
    Contexts ctx;
    const auto m1 = std::make_shared<MockPredicate>(1);
    const auto m2 = std::make_shared<MockPredicate>(1);
    const PredAnd<int> pred_and({m1});
    const auto r1 = pred_and.product(m2);
	std::stringstream ss;
	r1->print(ss);
	ASSERT_EQ("And(Mock(1), Mock(1))", ss.str());
    const auto m3 = std::make_shared<MockPredicate>(1);
    auto r2 = r1->product(m3);
	std::shared_ptr<const PredAnd<int>> po2 = std::dynamic_pointer_cast<const PredAnd<int>>(r2);
    ASSERT_EQ(3u, po2->size());
    ASSERT_TRUE(po2->select(1, ctx));
    ASSERT_FALSE(po2->select(2, ctx));
    ASSERT_TRUE(po2->select_out_of_context(1));
    ASSERT_FALSE(po2->select_out_of_context(2));
    r2 = r1->product(std::make_shared<MockPredicate>(2));
    po2 = std::dynamic_pointer_cast<const PredAnd<int>>(r2);
    ASSERT_EQ(3u, po2->size());
    ASSERT_FALSE(po2->select(1, ctx));
    ASSERT_FALSE(po2->select(2, ctx));
    ASSERT_FALSE(po2->select_out_of_context(1));
    ASSERT_FALSE(po2->select_out_of_context(2));

	const auto minact = std::make_shared<MockPredicate>(1, false);
	const PredAnd<int> pred_and2({ m1, minact });
	ASSERT_FALSE(pred_and2.active(Date(1989, 6, 4)));
}

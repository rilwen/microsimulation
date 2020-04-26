// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/predicate/pred_not.hpp"
#include "microsim-simulator/contexts.hpp"
#include "mock_predicate.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(PredNot, Negate) {
    Contexts ctx;
    const auto m1 = std::make_shared<MockPredicate>(1);
    auto n1 = std::make_shared<PredNot<int>>(m1);
    auto n2 = n1->negate();
	std::stringstream ss;
	n1->print(ss);
	ASSERT_EQ("Not(Mock(1))", ss.str());
    ASSERT_EQ(m1, n2);
	ASSERT_FALSE(n1->active(Date(2000, 1, 1)));
}

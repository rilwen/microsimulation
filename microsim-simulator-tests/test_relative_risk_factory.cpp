// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/relative_risk_factory.hpp"
#include "microsim-simulator/contexts.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(RelativeRiskFactory, Constant) {
    const RelativeRiskValueUnbound rvu(0.4, Period(PeriodType::YEARS, 5));
    std::unique_ptr<const RelativeRisk<int>> rr(RelativeRiskFactory::make_constant<int>(rvu));
    ASSERT_NE(nullptr, rr);
    Contexts ctx;
    ASSERT_EQ(rvu, rr->calc_relative_risk_unbound(-2, ctx));
    ASSERT_TRUE(rr->requires().empty());
}

#include <gtest/gtest.h>
#include "microsim-core/relative_risk_value.hpp"
#include <limits>

using namespace averisera;
using namespace averisera::microsim;

TEST(RelativeRiskValue, NegativeValue) {
    ASSERT_THROW(RelativeRiskValue(-0.3, Date(2012, 4, 1), Date(2013, 4, 1), RelativeRiskValue::Type::SCALABLE), std::domain_error);
}

TEST(RelativeRiskValue, NegativeInfiniteValue) {
    ASSERT_THROW(RelativeRiskValue(-std::numeric_limits<double>::infinity(), Date(2012, 4, 1), Date(2013, 4, 1), RelativeRiskValue::Type::SCALABLE), std::domain_error);
}

TEST(RelativeRiskValue, PositiveInfiniteValue) {
    ASSERT_THROW(RelativeRiskValue(std::numeric_limits<double>::infinity(), Date(2012, 4, 1), Date(2013, 4, 1), RelativeRiskValue::Type::SCALABLE), std::domain_error);
}

TEST(RelativeRiskValue, NaNValue) {
    ASSERT_THROW(RelativeRiskValue(std::numeric_limits<double>::quiet_NaN(), Date(2012, 4, 1), Date(2013, 4, 1), RelativeRiskValue::Type::SCALABLE), std::domain_error);
}


TEST(RelativeRiskValue, DatesOutOfOrder) {
    ASSERT_THROW(RelativeRiskValue(0.3, Date(2013, 4, 1), Date(2012, 4, 1), RelativeRiskValue::Type::SCALABLE), std::domain_error);
}

TEST(RelativeRiskValue, DatesEqual) {
    ASSERT_NO_THROW(RelativeRiskValue(0.3, Date(2013, 4, 1), Date(2013, 4, 1), RelativeRiskValue::Type::SCALABLE));
}

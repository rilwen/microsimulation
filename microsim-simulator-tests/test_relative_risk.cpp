#include <gtest/gtest.h>
#include "core/daycount.hpp"
#include "microsim-simulator/feature.hpp"
#include "microsim-simulator/relative_risk.hpp"
#include "microsim-core/schedule_definition.hpp"

using namespace averisera;
using namespace averisera::microsim;

static const RelativeRiskValueUnbound RR(1.51, Period(PeriodType::YEARS, 5));

class MockRelativeRisk : public RelativeRisk<int> {
public:
    RelativeRiskValueUnbound calc_relative_risk_unbound(const int&, const Contexts&) const override {
        return RR;
    }

    const FeatureUser<Feature>::feature_set_t& requires() const override {
        return Feature::empty();
    }
};

TEST(RelativeRisk, Test) {
    const MockRelativeRisk rr;
    ScheduleDefinition def;
    def.start = Date(2013, 11, 30);
    def.end = Date(2014, 12, 10);
    def.frequency.type = PeriodType::MONTHS;
    def.frequency.size = 3;
    def.daycount = Daycount::YEAR_FRACT();
    const Schedule schedule(def);
    const auto im_ctx = std::make_shared<ImmutableContext>(schedule, Ethnicity::IndexConversions());
    const Contexts ctx(im_ctx, std::make_shared<MutableContext>());
    const RelativeRiskValue v = rr(-1, ctx);
    ASSERT_EQ(RR.relative_risk, v.relative_risk);
    ASSERT_EQ(schedule.date(0), v.ref_start);
    ASSERT_EQ(schedule.date(0) + RR.period, v.ref_end);
}

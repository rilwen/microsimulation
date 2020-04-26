// (C) Averisera Ltd 2014-2020
#include "microsim-core/anchored_hazard_curve.hpp"
#include "microsim-core/hazard_curve.hpp"
#include "microsim-core/hazard_curve_factory.hpp"
#include "microsim-core/hazard_model.hpp"
#include "microsim-core/relative_risk_value.hpp"
#include "core/daycount.hpp"
#include "core/period.hpp"
#include <gtest/gtest.h>

using namespace averisera;
using namespace averisera::microsim;

static const auto DAYCOUNT = Daycount::DAYS_365();
static const auto FACTORY = HazardCurveFactory::PIECEWISE_CONSTANT();

TEST(HazardModel, Constructor) {
    std::vector<std::shared_ptr<const AnchoredHazardCurve>> curves(3);
    const Date start(2015, 1, 1);
    const Date end(2016, 1, 1);
    curves[0] = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({end}), std::vector<double>({0.01}), std::vector<HazardRateMultiplier>());
    curves[1] = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({end}), std::vector<double>({0.1}), std::vector<HazardRateMultiplier>());
    HazardModel model(curves, {1, 2, 2});
    ASSERT_EQ(3u, model.dim());
    ASSERT_EQ(start, model.start());
    ASSERT_EQ(1u, model.next_state(0));
    ASSERT_EQ(2u, model.next_state(1));
    ASSERT_EQ(2u, model.next_state(2));
}

TEST(HazardModel, Probability) {
    std::vector<std::shared_ptr<const AnchoredHazardCurve>> curves(2);
    const Date start(2015, 1, 1);
    const double h = 0.01;
    curves[0] = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({start + Period::years(1)}), std::vector<double>({1 - exp(-h)}), std::vector<HazardRateMultiplier>());
    HazardModel model(curves, {1, 1});
    const Period period = Period(PeriodType::DAYS, 2);
    ASSERT_EQ(0, model.calc_transition_probability(1, start, start + period));
    ASSERT_EQ(0, model.calc_transition_probability(1, start, start + period, 1.4));
    ASSERT_THROW(model.calc_transition_probability(0, start, start - period), std::out_of_range);
    ASSERT_THROW(model.calc_transition_probability(0, start - period, start + period), std::out_of_range);
    ASSERT_THROW(model.calc_transition_probability(0, start, start, -0.2), std::out_of_range);
    ASSERT_EQ(0., model.calc_transition_probability(0, start, start));
    ASSERT_EQ(0., model.calc_transition_probability(0, start, start + Period(PeriodType::YEARS, 2), 0.));
    const double expected_proba = 1 - exp(-1.2*h*30.0 / 365.0);
    const Date end = start + Period(PeriodType::DAYS, 30);
    const double multi = 1.2;
    ASSERT_NEAR(expected_proba, model.calc_transition_probability(0, start, end, multi), 1E-15);
    ASSERT_EQ(end, model.calc_end_date(0, start, expected_proba, multi));
    const Date end2 = end + Period(PeriodType::DAYS, 30);
    ASSERT_EQ(end2, model.calc_end_date(0, end, expected_proba, multi));
    ASSERT_EQ(Date::POS_INF, model.calc_end_date(1, start, expected_proba, multi));
    ASSERT_EQ(Date::POS_INF, model.calc_end_date(0, start, expected_proba, 0.));
}

TEST(HazardModel, RelativeRisk) {
    std::vector<std::shared_ptr<const AnchoredHazardCurve>> curves(2);
    const double h = 0.01;
    const Date start(2015, 1, 1);
    curves[0] = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({start + Period::years(1)}), std::vector<double>({h}), std::vector<HazardRateMultiplier>());
    HazardModel model(curves, {1, 1});
    Date t1(2015, 4, 1);
    Date t2(2015, 8, 20);
    const double rr = 0.5;
    const RelativeRiskValue rrv(rr, t1, t2, RelativeRiskValue::Type::SCALABLE);
    HazardRateMultiplier multi = model.calc_hazard_rate_multiplier(1, rrv);
    ASSERT_EQ(HazardRateMultiplier(1.0, Date::NEG_INF, Date::POS_INF, true), multi);
    multi = model.calc_hazard_rate_multiplier(0, rrv);
    ASSERT_TRUE(multi.value < 1);
    ASSERT_TRUE(multi.value > 0);
    ASSERT_TRUE(multi.from.is_neg_infinity());
    ASSERT_TRUE(multi.to.is_pos_infinity());
    double base = model.calc_transition_probability(0, t1, t2);
    double obs = model.calc_transition_probability(0, t1, t2, multi);
    ASSERT_NEAR(rr * base, obs, 1E-15);
    ASSERT_NEAR(obs, model.calc_transition_probability(0, t1, t2, rrv), 1E-15);

    t1 = start + Period(PeriodType::DAYS, 100);
    t2 = start + Period(PeriodType::DAYS, 200);
    const RelativeRiskValue rrv2(rr, start, t1, RelativeRiskValue::Type::SCALABLE);
    multi = model.calc_hazard_rate_multiplier(0, rrv2);
    base = model.calc_transition_probability(0, start, t1);
    obs = model.calc_transition_probability(0, start, t2, multi);
    ASSERT_NEAR(1 - pow(1 - rr * base, 2), obs, 1E-15);
    const RelativeRiskValue rrv3(rr, start, t1, RelativeRiskValue::Type::FIXED);
    multi = model.calc_hazard_rate_multiplier(0, rrv3);
    base = model.calc_transition_probability(0, start, t1);
    obs = model.calc_transition_probability(0, start, t2, multi);
    ASSERT_NEAR(1 - (1 - rr * base) * (1 - base), obs, 1E-15);
}


TEST(HazardModel, ZeroRisk) {
    std::vector<std::shared_ptr<const AnchoredHazardCurve>> curves(2);
    const double h = 0.0;
    const Date start(2015, 1, 1);
    curves[0] = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({start + Period::years(1)}), std::vector<double>({h}), std::vector<HazardRateMultiplier>());
    HazardModel model(curves, {1, 1});
    const Period period = Period(PeriodType::DAYS, 2);
    ASSERT_EQ(0, model.calc_transition_probability(1, start, start + period));
    ASSERT_EQ(0, model.calc_transition_probability(1, start, start + period, 1.4));    
    ASSERT_EQ(Date::POS_INF, model.calc_end_date(0, start, 0.01, 1.3));
    ASSERT_EQ(Date::POS_INF, model.calc_end_date(0, start, 0.01, 0.));
    ASSERT_EQ(start, model.calc_end_date(0, start, 0, 1));
}

TEST(HazardModel, ZeroRiskWithYearFract) {
    std::vector<std::shared_ptr<const AnchoredHazardCurve>> curves(2);
    const double h = 0.0;
    const Date start(2015, 1, 1);
    curves[0] = AnchoredHazardCurve::build(start, Daycount::YEAR_FRACT(), FACTORY, std::vector<Date>({start + Period::years(1)}), std::vector<double>({h}), std::vector<HazardRateMultiplier>());
    HazardModel model(curves, {1, 1});
    const Period period = Period(PeriodType::DAYS, 2);
    ASSERT_EQ(0, model.calc_transition_probability(1, start, start + period));
    ASSERT_EQ(0, model.calc_transition_probability(1, start, start + period, 1.4));    
    ASSERT_EQ(Date::POS_INF, model.calc_end_date(0, start, 0.01, 1.3));
    ASSERT_EQ(Date::POS_INF, model.calc_end_date(0, start, 0.01, 0.));
    ASSERT_EQ(start, model.calc_end_date(0, start, 0, 1));
}

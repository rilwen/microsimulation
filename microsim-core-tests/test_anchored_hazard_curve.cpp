// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/daycount.hpp"
#include "core/period.hpp"
#include "microsim-core/anchored_hazard_curve.hpp"
#include "microsim-core/hazard_curve.hpp"
#include "microsim-core/hazard_curve_factory.hpp"
#include "microsim-core/relative_risk_value.hpp"
#include <limits>

using namespace averisera;
using namespace averisera::microsim;

static const auto DAYCOUNT = Daycount::DAYS_365();
static const auto FACTORY = HazardCurveFactory::PIECEWISE_CONSTANT();


TEST(AnchoredHazardCurve, ZeroRisk) {
    const Date start(2015, 1, 1);
    const Date end(2016, 1, 1);
    const auto curve = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({end}), std::vector<double>({0.0}), std::vector<HazardRateMultiplier>());
    ASSERT_EQ(curve->jump_probability(end), 0.0);
    ASSERT_GT(curve->calc_next_date(end, 0.1), end);
    ASSERT_TRUE(curve->calc_next_date(end, 0.1).is_pos_infinity());
}

TEST(AnchoredHazardCurve, VeryLowRisk) {
    const Date start(2015, 1, 1);
    const Date end(2016, 1, 1);
    const double p = 1E-12;
    const auto curve = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({end}), std::vector<double>({p}), std::vector<HazardRateMultiplier>());
    ASSERT_NEAR(curve->jump_probability(end), p, 1E-24);
    ASSERT_GT(curve->calc_next_date(end, 0.1), end);
    Date result = curve->calc_next_date(end, 0.1);
    ASSERT_EQ(Date::MAX, result) << result;
}

TEST(AnchoredHazardCurve, Probability) {
    const Date start(2015, 1, 1);
    const Date end(2016, 1, 1);
    const double jump_proba = 0.34;
    const auto daycount = Daycount::YEAR_FRACT();
    const auto curve = AnchoredHazardCurve::build(start, daycount, FACTORY, std::vector<Date>({end}), std::vector<double>({jump_proba}), std::vector<HazardRateMultiplier>());
    ASSERT_NEAR(curve->jump_probability(end), jump_proba, 1E-15);
    const double ihr = HazardCurve::integrated_hazard_rate_from_jump_proba(jump_proba);
	ASSERT_NEAR(ihr, curve->average_hazard_rate(start, end), 1E-15);
	ASSERT_NEAR(0.4 * ihr, curve->integrated_hazard_rate(start, end, HazardRateMultiplier(0.4)), 1E-15);
	ASSERT_NEAR(0.4 * ihr, curve->integrated_hazard_rate(start, end, HazardRateMultiplier(0.4, start, end, true)), 1E-15);
	ASSERT_NEAR(ihr, curve->average_hazard_rate(start, start), 1E-15);
    ASSERT_EQ(curve->calc_next_date(start, ihr), end);
    ASSERT_NEAR(curve->conditional_jump_probability(end, end + Period::years(1)), jump_proba, 1E-15);
	const auto moved = curve->move(end);	
}

TEST(AnchoredHazardCurve, ProbabilityMultiplier) {
	const Date start(2015, 1, 1);
	const Date end(2016, 1, 1);
	const double jump_proba = 0.34;
	const auto daycount = Daycount::DAYS_365();
	const auto curve = AnchoredHazardCurve::build(start, daycount, FACTORY, std::vector<Date>({ end }), std::vector<double>({ jump_proba }), std::vector<HazardRateMultiplier>({HazardRateMultiplier(2.0)}));
	const double ihr = HazardCurve::integrated_hazard_rate_from_jump_proba(jump_proba);	
	ASSERT_NEAR(2 * ihr, curve->average_hazard_rate(start, end), 1E-15);
}

TEST(AnchoredHazardCurve, ProbabilityMultiplier2) {
	const Date start(2015, 1, 1);
	const Date end(2016, 1, 1);
	const double jump_proba = 0.34;
	const auto daycount = Daycount::DAYS_365();
	const auto curve = AnchoredHazardCurve::build(start, daycount, FACTORY, std::vector<Date>({ end }), std::vector<double>({ jump_proba }), std::vector<HazardRateMultiplier>({ HazardRateMultiplier(2.0, start, end, true) }));
	const double ihr = HazardCurve::integrated_hazard_rate_from_jump_proba(jump_proba);
	ASSERT_NEAR(2 * ihr, curve->average_hazard_rate(start, end), 1E-15);
	ASSERT_NEAR(ihr, curve->average_hazard_rate(end, end + (end - start)), 1E-15);
	const auto moved = curve->move(end);
	ASSERT_NEAR(2 * ihr, moved->average_hazard_rate(end, end + (end - start)), 1E-15);
}

TEST(AnchoredHazardCurve, ProbabilityMultiplier3) {
	const Date start(2015, 1, 1);
	const Date end(2016, 1, 1);
	const double jump_proba = 0.34;
	const auto daycount = Daycount::DAYS_365();
	const auto curve = AnchoredHazardCurve::build(start, daycount, FACTORY, std::vector<Date>({ end }), std::vector<double>({ jump_proba }), std::vector<HazardRateMultiplier>({ HazardRateMultiplier(2.0, start, end, false) }));
	const double ihr = HazardCurve::integrated_hazard_rate_from_jump_proba(jump_proba);
	ASSERT_NEAR(2 * ihr, curve->average_hazard_rate(start, end), 1E-15);
	ASSERT_NEAR(ihr, curve->average_hazard_rate(end, end + (end - start)), 1E-15);
	const auto moved = curve->move(end);
	ASSERT_NEAR(ihr, moved->average_hazard_rate(end, end + (end - start)), 1E-15);
}

TEST(AnchoredHazardCurve, MoveDates) {
    const Date curve_start(2010, 1, 1);
    std::shared_ptr<const AnchoredHazardCurve> curve = AnchoredHazardCurve::build(curve_start, DAYCOUNT, FACTORY, std::vector<Date>({curve_start + Period::years(1)}), std::vector<double>({0.01}), std::vector<HazardRateMultiplier>());
    const Date new_start(2015, 10, 4);
    const auto moved = curve->move(new_start);
    ASSERT_NE(nullptr, moved);
    ASSERT_EQ(new_start, moved->start());
    ASSERT_NEAR(curve->integrated_hazard_rate(curve_start, curve_start + Period::days(200)), moved->integrated_hazard_rate(new_start, new_start + Period::days(200)), 1E-14);
}

TEST(AnchoredHazardCurve, MovePeriodsAdditive) {
    const Date curve_start(2010, 1, 1);
    std::shared_ptr<const AnchoredHazardCurve> curve = AnchoredHazardCurve::build(curve_start, DAYCOUNT, FACTORY, std::vector<Period>({Period::days(90), Period::days(180)}), std::vector<double>({0.001, 0.01}), true, false, std::vector<HazardRateMultiplier>());
    const Date new_start(2015, 10, 4);
    const auto moved = curve->move(new_start);
    ASSERT_NE(nullptr, moved);
    ASSERT_EQ(new_start, moved->start());
	ASSERT_NEAR(0.01, curve->jump_probability(curve_start + Period::days(270)), 1E-15);
    ASSERT_NEAR(curve->integrated_hazard_rate(curve_start, curve_start + Period::days(200)), moved->integrated_hazard_rate(new_start, new_start + Period::days(200)), 1E-14);
}

TEST(AnchoredHazardCurve, MovePeriodsAdditiveConditional) {
	const Date curve_start(2010, 1, 1);
	std::shared_ptr<const AnchoredHazardCurve> curve = AnchoredHazardCurve::build(curve_start, DAYCOUNT, FACTORY, std::vector<Period>({ Period::days(90), Period::days(180) }), std::vector<double>({ 0.001, (0.01 - 0.001) / (1 - 0.001) }), true, true, std::vector<HazardRateMultiplier>());
	const Date new_start(2015, 10, 4);
	const auto moved = curve->move(new_start);
	ASSERT_NE(nullptr, moved);
	ASSERT_NEAR(0.01, curve->jump_probability(curve_start + Period::days(270)), 1E-15);
	ASSERT_EQ(new_start, moved->start());
	ASSERT_NEAR(curve->integrated_hazard_rate(curve_start, curve_start + Period::days(200)), moved->integrated_hazard_rate(new_start, new_start + Period::days(200)), 1E-14);
}

TEST(AnchoredHazardCurve, MovePeriodsNonAdditive) {
    const Date curve_start(2010, 1, 1);
    std::shared_ptr<const AnchoredHazardCurve> curve = AnchoredHazardCurve::build(curve_start, DAYCOUNT, FACTORY, std::vector<Period>({Period::days(90), Period::days(180)}), std::vector<double>({0.001, 0.01}), false, false, std::vector<HazardRateMultiplier>());
    const Date new_start(2015, 10, 4);
    const auto moved = curve->move(new_start);
    ASSERT_NE(nullptr, moved);
    ASSERT_EQ(new_start, moved->start());
    ASSERT_NEAR(curve->integrated_hazard_rate(curve_start, curve_start + Period::days(200)), moved->integrated_hazard_rate(new_start, new_start + Period::days(200)), 1E-14);
}

TEST(AnchoredHazardCurve, RelativeRisk) {
    const double h = 0.01;
    const Date start(2015, 1, 1);
    std::shared_ptr<const AnchoredHazardCurve> curve = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({start + Period::years(1)}), std::vector<double>({h}), std::vector<HazardRateMultiplier>());
    Date t1(2015, 4, 1);
    Date t2(2015, 8, 20);
    const double rr = 0.5;
    const RelativeRiskValue rrv(rr, t1, t2, RelativeRiskValue::Type::SCALABLE);
    HazardRateMultiplier multi = curve->calc_hazard_rate_multiplier(rrv);
    ASSERT_TRUE(multi.value < 1);
    ASSERT_TRUE(multi.value > 0);
    ASSERT_TRUE(multi.from.is_neg_infinity());
    ASSERT_TRUE(multi.to.is_pos_infinity());
    double base = curve->conditional_jump_probability(t1, t2);
    double obs = curve->conditional_jump_probability(t1, t2, multi);
    ASSERT_NEAR(rr * base, obs, 1E-15);
    ASSERT_EQ(obs, curve->conditional_jump_probability(t1, t2, rrv));

    t1 = start + Period(PeriodType::DAYS, 100);
    t2 = start + Period(PeriodType::DAYS, 200);
    const RelativeRiskValue rrv2(rr, start, t1, RelativeRiskValue::Type::SCALABLE);
    multi = curve->calc_hazard_rate_multiplier(rrv2);
    base = curve->conditional_jump_probability(start, t1);
    obs = curve->conditional_jump_probability(start, t2, multi);
    ASSERT_NEAR(1 - pow(1 - rr * base, 2), obs, 1E-15);
    ASSERT_EQ(obs, curve->conditional_jump_probability(start, t2, rrv2));
    const RelativeRiskValue rrv3(rr, start, t1, RelativeRiskValue::Type::FIXED);
    multi = curve->calc_hazard_rate_multiplier(rrv3);
    base = curve->conditional_jump_probability(start, t1);
    obs = curve->conditional_jump_probability(start, t2, multi);
    ASSERT_NEAR(1 - (1 - rr * base) * (1 - base), obs, 1E-15);
    ASSERT_EQ(obs, curve->conditional_jump_probability(start, t2, rrv3));
}

TEST(AnchoredHazardCurve, IntegratedHazardRate) {
    const double h = 0.01;
    const Date start(2015, 1, 1);
    const Date t1 = start + Period::years(1);
    const double jp = 1 - exp(-h * DAYCOUNT->calc(start, t1));
    std::shared_ptr<const AnchoredHazardCurve> curve = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({t1}), std::vector<double>({jp}), std::vector<HazardRateMultiplier>());
    const Date t2 = t1 + Period::days(100);
    double ihr = curve->integrated_hazard_rate(t1, t2);
    ASSERT_NEAR(h * DAYCOUNT->calc(t1, t2), ihr, 1E-16);
    ihr = curve->integrated_hazard_rate(t1, t2, HazardRateMultiplier(2.0));
    ASSERT_NEAR(2 * h * DAYCOUNT->calc(t1, t2), ihr, 1E-16);
    ihr = curve->integrated_hazard_rate(t1, t2, HazardRateMultiplier(2.0, start, t1 + Period::days(50), false));
    ASSERT_NEAR(1.5 * h * DAYCOUNT->calc(t1, t2), ihr, 1E-16);
    ihr = curve->integrated_hazard_rate(t1, t2, HazardRateMultiplier(2.0, t1 + Period::days(50), Date::POS_INF, true));
    ASSERT_NEAR(1.5 * h * DAYCOUNT->calc(t1, t2), ihr, 1E-16);
    curve = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({t1, t2}), std::vector<double>({jp, jp}), std::vector<HazardRateMultiplier>());
    ASSERT_NEAR(0.0, curve->integrated_hazard_rate(t2, t2 + Period::days(500)), 1E-16);
    ASSERT_NEAR(0.0, curve->integrated_hazard_rate(t2, t2 + Period::days(500), HazardRateMultiplier(10.0, t1, t1 + Period::weeks(1), false)), 1E-16);
    ihr = curve->integrated_hazard_rate(t1 - Period::days(50), t1 + Period::days(50), HazardRateMultiplier(2.0, t1 - Period::days(50), t1 + Period::days(50), true));
    ASSERT_NEAR(2 * h * DAYCOUNT->calc(t1 - Period::days(50), t1), ihr, 1E-16);
}

TEST(AnchoredHazardCurve, IntegratedHazardRateManyMultipliers) {
    const double h = 0.01;
    const Date start(2015, 1, 1);
    const Date t1 = start + Period::years(1);
    const double jp = 1 - exp(-h * DAYCOUNT->calc(start, t1));
    std::shared_ptr<const AnchoredHazardCurve> curve = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({t1}), std::vector<double>({jp}), std::vector<HazardRateMultiplier>());
    const Date t2 = t1 + Period::days(100);
    std::vector<HazardRateMultiplier> multis({
            HazardRateMultiplier(2.0),
                HazardRateMultiplier(),
                HazardRateMultiplier(2.0, start, t1 - Period::days(50), true),
                HazardRateMultiplier(0.5, t1, t1, false),
                HazardRateMultiplier(1.5, t1 + Period::days(50), Date::POS_INF, true)
                });
    ASSERT_EQ(curve->integrated_hazard_rate(start, t2), curve->integrated_hazard_rate(start, t2, std::vector<HazardRateMultiplier>()));
    for (HazardRateMultiplier multi: multis) {
        ASSERT_EQ(curve->integrated_hazard_rate(start, t2, multi), curve->integrated_hazard_rate(start, t2, std::vector<HazardRateMultiplier>(1, multi)));
    }
    ASSERT_NEAR(h * (4 * DAYCOUNT->calc(start, t1 - Period::days(50)) + 2 * DAYCOUNT->calc(t1 - Period::days(50), t1 + Period::days(50)) + 3 * DAYCOUNT->calc(t1 + Period::days(50), t2)),
                curve->integrated_hazard_rate(start, t2, multis), 5E-16);
    curve = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({t1, t2}), std::vector<double>({jp, jp}), std::vector<HazardRateMultiplier>());
    for (HazardRateMultiplier multi: multis) {
        ASSERT_EQ(curve->integrated_hazard_rate(start, t2, multi), curve->integrated_hazard_rate(start, t2, std::vector<HazardRateMultiplier>(1, multi)));
    }
    ASSERT_NEAR(h * (4 * DAYCOUNT->calc(start, t1 - Period::days(50)) + 2 * DAYCOUNT->calc(t1 - Period::days(50), t1)),
                curve->integrated_hazard_rate(start, t2, multis), 5E-16);
    ASSERT_THROW(curve->integrated_hazard_rate(start, t2, std::vector<HazardRateMultiplier>(100, multis[0])), std::runtime_error) << "Prevent excessive recursion";
}

TEST(AnchoredHazardCurve, DivideByHazardRateMultiplier) {
    const double h = 0.01;
    const Date start(2015, 1, 1);
    const Date t1 = start + Period::years(1);
    const double jp = 1 - exp(-h * DAYCOUNT->calc(start, t1));
    std::shared_ptr<const AnchoredHazardCurve> curve = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({t1}), std::vector<double>({jp}), std::vector<HazardRateMultiplier>());
    const Date t2 = t1 + Period::days(100);
    double base_ihr = curve->integrated_hazard_rate(t1, t2);
    ASSERT_EQ(0.0, curve->divide_by_hazard_rate_multiplier(t1, 0.0, HazardRateMultiplier(10.0)));
    HazardRateMultiplier multi;
    ASSERT_EQ(base_ihr, curve->divide_by_hazard_rate_multiplier(t1, base_ihr, multi));
    multi = HazardRateMultiplier(2.0);
    ASSERT_NEAR(base_ihr / 2.0, curve->divide_by_hazard_rate_multiplier(t1, base_ihr, multi), 1E-16);
    multi = HazardRateMultiplier(2.0, start, t1 + Period::days(50), true);
    ASSERT_NEAR(base_ihr, curve->divide_by_hazard_rate_multiplier(t1, curve->integrated_hazard_rate(t1, t2, multi), multi), 1E-16);
    multi = HazardRateMultiplier(2.0, t1 + Period::days(50), Date::POS_INF, false);
    ASSERT_NEAR(1E-5, curve->divide_by_hazard_rate_multiplier(start, 1E-5, multi), 1E-15);
    ASSERT_NEAR(base_ihr, curve->divide_by_hazard_rate_multiplier(t1, curve->integrated_hazard_rate(t1, t2, multi), multi), 1E-16);
    ASSERT_NEAR(curve->integrated_hazard_rate(start, start + Period::days(2)), curve->divide_by_hazard_rate_multiplier(start, curve->integrated_hazard_rate(start, start + Period::days(2), multi), multi), 1E-16);
    multi = HazardRateMultiplier(3.0, t1, t2, false);
    ASSERT_NEAR(1E-5, curve->divide_by_hazard_rate_multiplier(start, 1E-5, multi), 1E-16);
    ASSERT_NEAR(1E-5, curve->divide_by_hazard_rate_multiplier(t2, 1E-5, multi), 1E-16);
    ASSERT_NEAR(1E-5, curve->divide_by_hazard_rate_multiplier(t1, 3E-5, multi), 1E-16);
    ASSERT_NEAR(curve->integrated_hazard_rate(t1 - Period::days(50), t2), curve->divide_by_hazard_rate_multiplier(t1 - Period::days(50), curve->integrated_hazard_rate(t1 - Period::days(50), t2, multi), multi), 1E-16);
    curve = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({t1, t2}), std::vector<double>({jp, jp}), std::vector<HazardRateMultiplier>());
    base_ihr = curve->integrated_hazard_rate(start, t2);
    multi = HazardRateMultiplier(2.0, t1 - Period::days(50), t1 + Period::days(50), true);
    ASSERT_NEAR(base_ihr, curve->divide_by_hazard_rate_multiplier(start, curve->integrated_hazard_rate(start, t2, multi), multi), 1E-16);
    ASSERT_NEAR(curve->integrated_hazard_rate(t1, t2), curve->divide_by_hazard_rate_multiplier(t1, curve->integrated_hazard_rate(t1, t2, multi), multi), 1E-16);
    ASSERT_NEAR(curve->integrated_hazard_rate(t1 - Period::days(50), t2), curve->divide_by_hazard_rate_multiplier(t1 - Period::days(50), curve->integrated_hazard_rate(t1 - Period::days(50), t2, multi), multi), 1E-16);
}

TEST(AnchoredHazardCurve, DivideByHazardRateMultiplierManyMultipliers) {
    const double h = 0.01;
    const Date start(2015, 1, 1);
    const Date t1 = start + Period::years(1);
    const double jp = 1 - exp(-h * DAYCOUNT->calc(start, t1));
    std::shared_ptr<const AnchoredHazardCurve> curve = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({t1}), std::vector<double>({jp}), std::vector<HazardRateMultiplier>());
    const Date t2 = t1 + Period::days(100);
    double base_ihr = curve->integrated_hazard_rate(t1, t2);
    std::vector<HazardRateMultiplier> multis({
            HazardRateMultiplier(2.0)
                , HazardRateMultiplier()
                , HazardRateMultiplier(0.5, t1, t1, false)
                , HazardRateMultiplier(2.0, start, t1 - Period::days(50), false)
                , HazardRateMultiplier(1.5, t1 + Period::days(50), Date::POS_INF, false)
                });
    ASSERT_EQ(base_ihr, curve->divide_by_hazard_rate_multiplier(start, base_ihr, std::vector<HazardRateMultiplier>()));
    for (HazardRateMultiplier multi: multis) {
        const std::vector<HazardRateMultiplier> multis1(1, multi);
        ASSERT_EQ(curve->divide_by_hazard_rate_multiplier(start, base_ihr, multi), curve->divide_by_hazard_rate_multiplier(start, base_ihr, multis1));
        ASSERT_EQ(curve->divide_by_hazard_rate_multiplier(t1, base_ihr, multi), curve->divide_by_hazard_rate_multiplier(t1, base_ihr, multis1));
    }
    ASSERT_NEAR(base_ihr, curve->divide_by_hazard_rate_multiplier(t1, curve->integrated_hazard_rate(t1, t2, multis), multis), 5E-16);
    ASSERT_NEAR(curve->integrated_hazard_rate(start, t2), curve->divide_by_hazard_rate_multiplier(start, curve->integrated_hazard_rate(start, t2, multis), multis), 5E-16);
    ASSERT_NEAR(curve->integrated_hazard_rate(t1 - Period::days(50), t2), curve->divide_by_hazard_rate_multiplier(t1 - Period::days(50), curve->integrated_hazard_rate(t1 - Period::days(50), t2, multis), multis), 1E-16);
    curve = AnchoredHazardCurve::build(start, DAYCOUNT, FACTORY, std::vector<Date>({t1, t2}), std::vector<double>({jp, jp}), std::vector<HazardRateMultiplier>());
    base_ihr = curve->integrated_hazard_rate(start, t2);
    for (HazardRateMultiplier multi: multis) {
        ASSERT_EQ(curve->divide_by_hazard_rate_multiplier(start, base_ihr, multi), curve->divide_by_hazard_rate_multiplier(start, base_ihr, std::vector<HazardRateMultiplier>(1, multi)));
    }
    ASSERT_NEAR(base_ihr, curve->divide_by_hazard_rate_multiplier(start, curve->integrated_hazard_rate(start, t2, multis), multis), 5E-16);
    ASSERT_NEAR(curve->integrated_hazard_rate(t1, t2), curve->divide_by_hazard_rate_multiplier(t1, curve->integrated_hazard_rate(t1, t2, multis), multis), 5E-16);
    ASSERT_NEAR(curve->integrated_hazard_rate(t1 - Period::days(50), t2), curve->divide_by_hazard_rate_multiplier(t1 - Period::days(50), curve->integrated_hazard_rate(t1 - Period::days(50), t2, multis), multis), 1E-16);
    ASSERT_THROW(curve->divide_by_hazard_rate_multiplier(start, 1E-3, std::vector<HazardRateMultiplier>(100, multis[0])), std::runtime_error) << "Prevent excessive recursion";
}

TEST(AnchoredHazardCurve, SafeDivide) {
    ASSERT_NEAR(5.0, AnchoredHazardCurve::safe_divide(10.0, 2.0), 1E-15);
    ASSERT_EQ(0.0, AnchoredHazardCurve::safe_divide(0.0, 0.0));
    ASSERT_EQ(std::numeric_limits<double>::infinity(), AnchoredHazardCurve::safe_divide(1.0, 0.0));
    ASSERT_EQ(1E-12, AnchoredHazardCurve::safe_divide(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()));
}

#include <gtest/gtest.h>
#include "microsim-simulator/operator/operator_enforcer.hpp"
#include "microsim-simulator/predicate_factory.hpp"
#include "microsim-simulator/dispatcher_factory.hpp"
#include "microsim-simulator/history_factory.hpp"
#include "microsim-simulator/operator_factory.hpp"
#include "microsim-simulator/immutable_context.hpp"
#include "microsim-simulator/history_registry.hpp"
#include "microsim-simulator/mutable_context.hpp"
#include "microsim-simulator/person.hpp"
#include "microsim-core/schedule.hpp"
#include "microsim-simulator/contexts.hpp"
#include "core/normal_distribution.hpp"
#include "testing/rng_precalc.hpp"

using namespace averisera;
using namespace averisera::microsim;

static std::vector<std::unique_ptr<History>> build_histories() {
    std::vector<std::unique_ptr<History>> hv;
    hv.push_back(HistoryFactory::DENSE<double>()(""));
    return hv;
}

static const auto DISPATCHER = ImmutableContext::person_history_dispatcher_ptr_t(DispatcherFactory::make_constant<Person>(HistoryFactory::DENSE<double>(), PredicateFactory::make_true<Person>()));

TEST(OperatorEnforcer, Test) {
    const Date d1(2012, 1, 1);
    const Date d2(2012, 6, 1);
    auto imm_ctx = std::make_shared<ImmutableContext>(Schedule({ d1, d2 }));
    const auto idx = imm_ctx->register_person_variable("X", DISPATCHER);
    auto mut_ctx = std::make_shared<MutableContext>(std::unique_ptr<RNG>(new averisera::testing::RNGPrecalc(std::vector<double>({ 0.4, 0.41, 0.45, 0.5 }))));
    Contexts ctx(imm_ctx, mut_ctx);
    auto p1 = std::make_shared<Person>(1, PersonAttributes(Sex::MALE, 0), Date(1990, 1, 1));
    auto p2 = std::make_shared<Person>(2, PersonAttributes(Sex::MALE, 0), Date(1990, 1, 1));
    p1->set_histories(build_histories());
    p2->set_histories(build_histories());
    const auto distr1 = std::make_shared<NormalDistribution>(0.1);
    const auto distr2 = std::make_shared<NormalDistribution>(0.2);
    const std::shared_ptr<const Predicate<Person> > pred = PredicateFactory::make_true<Person>();
    const std::shared_ptr<const Operator<Person>> op(OperatorFactory::make_enforcer<Person>("X", pred, { distr1, distr2 }, HistoryFactory::SPARSE<double>(), nullptr));
    ASSERT_EQ(1u, op->requirements().size());
    ASSERT_EQ("X", std::get<0>(op->requirements()[0]));
    ASSERT_EQ(HistoryFactory::SPARSE<double>(), std::get<1>(op->requirements()[0]));
    ASSERT_EQ(pred, std::get<2>(op->requirements()[0]));
    op->apply({ p1, p2 }, ctx);
    ctx.mutable_ctx().advance_date_index();
    op->apply({ p1, p2 }, ctx);
    ASSERT_NO_THROW(op->apply({ p1, p2 }, ctx)) << "We should be able to correct a distribution";
    ASSERT_EQ(d2, p1->history(idx).last_date());
    ASSERT_EQ(d2, p2->history(idx).last_date());
    ASSERT_NEAR(0.1 + NormalDistribution::normsinv(0.25), p1->history(idx).last_as_double(d1), 1E-14);
    ASSERT_NEAR(0.1 + NormalDistribution::normsinv(0.75), p2->history(idx).last_as_double(d1), 1E-14);
    ASSERT_NEAR(0.2 + NormalDistribution::normsinv(0.25), p1->history(idx).last_as_double(d2), 1E-14);
    ASSERT_NEAR(0.2 + NormalDistribution::normsinv(0.75), p2->history(idx).last_as_double(d2), 1E-14);
    op->apply({ p1 }, ctx);
    ASSERT_NEAR(0.2, p1->history(idx).last_as_double(d2), 1E-14);
    ASSERT_TRUE(op->is_active(d1));
    ASSERT_TRUE(op->is_active(d1));
}

TEST(OperatorEnforcer, CustomSchedule) {
    const Date d1(2012, 1, 1);
    const Date d2(2012, 6, 1);
    auto imm_ctx = std::make_shared<ImmutableContext>(Schedule({ d1, d2 }));
    const auto idx = imm_ctx->register_person_variable("X", DISPATCHER);
    auto mut_ctx = std::make_shared<MutableContext>(std::unique_ptr<RNG>(new averisera::testing::RNGPrecalc(std::vector<double>({ 0.4, 0.41, 0.45, 0.5 }))));
    Contexts ctx(imm_ctx, mut_ctx);
    auto p1 = std::make_shared<Person>(1, PersonAttributes(Sex::MALE, 0), Date(1990, 1, 1));
    auto p2 = std::make_shared<Person>(2, PersonAttributes(Sex::MALE, 0), Date(1990, 1, 1));
    p1->set_histories(build_histories());
    p2->set_histories(build_histories());
    const auto distr1 = std::make_shared<NormalDistribution>(0.1);
    const auto distr2 = std::make_shared<NormalDistribution>(0.2);
    const std::shared_ptr<const Predicate<Person> > pred = PredicateFactory::make_true<Person>();
    std::unique_ptr<Schedule> cust_schedule(new Schedule({d1}));
    ASSERT_EQ(1u, cust_schedule->nbr_dates());
    const std::shared_ptr<const Operator<Person>> op(OperatorFactory::make_enforcer<Person>("X", pred, { distr1 }, HistoryFactory::SPARSE<double>(), std::move(cust_schedule)));
    ASSERT_TRUE(op->is_active(d1));
    ASSERT_FALSE(op->is_active(d2));
    op->apply({ p1, p2 }, ctx);
    ASSERT_NEAR(0.1 + NormalDistribution::normsinv(0.25), p1->history(idx).last_as_double(d1), 1E-14);
    ASSERT_NEAR(0.1 + NormalDistribution::normsinv(0.75), p2->history(idx).last_as_double(d1), 1E-14);
    ctx.mutable_ctx().advance_date_index();
    ASSERT_THROW(op->apply({ p1, p2 }, ctx), std::exception);
}

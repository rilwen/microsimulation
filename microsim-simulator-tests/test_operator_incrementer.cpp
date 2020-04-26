// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/operator/operator_incrementer.hpp"
#include "microsim-simulator/dispatcher_factory.hpp"
#include "microsim-simulator/predicate_factory.hpp"
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

TEST(OperatorIncrementer, Test) {
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
    const auto op = OperatorFactory::make_incrementer<Person>("X", pred, { distr1, distr2 }, nullptr);
    ASSERT_EQ(0, op->requirements().size());
	ASSERT_EQ(1u, op->user_requirements().size());
    ASSERT_EQ("X", op->user_requirements()[0]);
    p1->history(idx).append(d1, 0.0);
    p2->history(idx).append(d1, 0.1);
    op->apply({ p1, p2 }, ctx);
    ctx.mutable_ctx().advance_date_index();
    op->apply({ p1, p2 }, ctx);
    ASSERT_EQ(d2, p1->history(idx).last_date());
    ASSERT_EQ(d2, p2->history(idx).last_date());
    ASSERT_NEAR(0.0, p1->history(idx).last_as_double(d1), 1E-14);
    ASSERT_NEAR(0.1, p2->history(idx).last_as_double(d1), 1E-14);
    ASSERT_NEAR(0.1 + NormalDistribution::normsinv(0.4), p1->history(idx).last_as_double(d2), 1E-14);
    ASSERT_NEAR(0.2 + NormalDistribution::normsinv(0.41), p2->history(idx).last_as_double(d2), 1E-14);
}

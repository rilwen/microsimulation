// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/immutable_context.hpp"
#include "microsim-simulator/dispatcher_factory.hpp"
#include "microsim-simulator/history_factory.hpp"
#include "microsim-simulator/predicate_factory.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(ImmutableContext, Histories) {
    ImmutableContext immctx;
    ASSERT_FALSE(immctx.person_history_registry().has_variable("FOO"));
    ASSERT_THROW(immctx.register_person_variable("FOO", nullptr), std::domain_error);
    const auto idx = immctx.register_person_variable("FOO", ImmutableContext::person_history_dispatcher_ptr_t(DispatcherFactory::make_constant<Person>(HistoryFactory::DENSE<double>(), PredicateFactory::make_true<Person>())));
    ASSERT_EQ(0u, idx);
}

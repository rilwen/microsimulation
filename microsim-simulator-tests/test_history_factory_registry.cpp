// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/dispatcher_factory.hpp"
#include "microsim-simulator/history_factory.hpp"
#include "microsim-simulator/history_factory_registry.hpp"
#include "microsim-simulator/person.hpp"
#include "microsim-simulator/predicate_factory.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(HistoryFactoryRegistry, Dispatchers) {
    HistoryFactoryRegistry<Person> registry;
    std::vector<std::shared_ptr<const Predicate<Person> > > predicates(2);
    predicates[0] = PredicateFactory::make_sex(Sex::MALE);
    predicates[1] = PredicateFactory::make_sex(Sex::FEMALE);
    std::vector<typename HistoryFactoryRegistry<Person>::factory_t> factories(2);
    factories[0] = HistoryFactory::DENSE<double>();
    factories[1] = HistoryFactory::SPARSE<int>();
    const HistoryFactoryRegistry<Person>::dispatcher_ptr_t dispatcher = DispatcherFactory::make_group(predicates, factories, Feature::empty());
    const auto idx = registry.register_variable("FOO", dispatcher);
    ASSERT_EQ(0u, idx);
    ASSERT_EQ(dispatcher, registry.variable_history_factory_dispatcher(idx));
    ASSERT_EQ(1u, registry.nbr_variables());
    ASSERT_THROW(registry.variable_history_factory_dispatcher(1), std::out_of_range);

    // provide factory only for male
    predicates.resize(1);
    factories.resize(1);
    const HistoryFactoryRegistry<Person>::dispatcher_ptr_t dispatcher2 = DispatcherFactory::make_group(predicates, factories, Feature::empty());
    const auto idx2 = registry.register_variable("BAR", dispatcher2);
    ASSERT_EQ(1u, idx2);
    ASSERT_EQ(dispatcher2, registry.variable_history_factory_dispatcher(idx2));
    ASSERT_EQ(2u, registry.nbr_variables());
    ASSERT_THROW(registry.register_variable("FOO", dispatcher), std::domain_error);
    const Date dob(1989, 6, 4);
    Person p(10, PersonAttributes(Sex::FEMALE, 0), dob);
    const auto histories = registry.make_histories(p);
    ASSERT_NE(nullptr, histories[0]);
    ASSERT_EQ(nullptr, histories[1]);
    Person p2(10, PersonAttributes(Sex::MALE, 0), dob);
    const auto histories2 = registry.make_histories(p2);
    ASSERT_NE(nullptr, histories2[0]);
    ASSERT_NE(nullptr, histories2[1]);
    const auto histories3 = registry.make_histories();
    ASSERT_EQ(2u, histories3.size());
    ASSERT_EQ(nullptr, histories3[0]);
    ASSERT_EQ(nullptr, histories3[1]);
}

TEST(HistoryFactoryRegistry, Common) {
    HistoryFactoryRegistry<Person> registry;
    registry.register_variable("BMI", HistoryFactory::DENSE<double>());
    registry.register_variable("smoking", HistoryFactory::SPARSE<uint8_t>());
    const auto histories3 = registry.make_histories();
    ASSERT_EQ(2u, histories3.size());
    ASSERT_NE(nullptr, histories3[0]);
    ASSERT_NE(nullptr, histories3[1]);
}

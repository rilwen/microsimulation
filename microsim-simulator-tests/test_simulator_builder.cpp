/*
(C) Averisera Ltd 2017
*/
#include <gtest/gtest.h>
#include "microsim-simulator/contexts.hpp"
#include "microsim-simulator/simulator_builder.hpp"
#include "microsim-simulator/simulator.hpp"
#include "microsim-simulator/predicate/pred_true.hpp"
#include "microsim-simulator/migration/migration_generator_dummy.hpp"
#include "microsim-core/schedule.hpp"
#include "mock_operator.hpp"
#include "mock_observer.hpp"

using namespace averisera;
using namespace averisera::microsim;

typedef MockOperator<PredTrue<Person>, Person> OurOperator;

TEST(SimulatorBuilder, Test) {
    SimulatorBuilder bldr;
    PredTrue<Person> pred;
    bldr.add_operator(std::make_shared<OurOperator>(true));
    ASSERT_THROW(bldr.add_operator(nullptr), std::domain_error);
    bldr.add_observer(std::make_shared<MockObserver>());
    ASSERT_THROW(bldr.add_observer(nullptr), std::domain_error);
	bldr.set_initial_population_size(1000).set_add_newborns(false);
	bldr.add_migration_generator(std::make_shared<MigrationGeneratorDummy>());
	const Schedule schedule({ Date(2000, 1, 1), Date(2001, 1, 1) });
    const Simulator sim(bldr.build(Contexts(schedule)));
	ASSERT_EQ(1000u, sim.initial_population_size());
	ASSERT_FALSE(sim.is_add_newborns());
	ASSERT_EQ(schedule, sim.simulation_schedule());
}

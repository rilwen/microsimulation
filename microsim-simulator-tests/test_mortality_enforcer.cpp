// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/contexts.hpp"
#include "microsim-simulator/operator/mortality_enforcer.hpp"
#include "microsim-simulator/person.hpp"
#include "microsim-simulator/predicate_factory.hpp"
#include "microsim-core/person_attributes.hpp"
#include "core/generic_distribution_bool.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(MortalityEnforcer, SingleDate) {
	const size_t n_alive = 80;
	const size_t n_dead = 20;
	const double p_death = 0.25;
	const std::vector<std::shared_ptr<const GenericDistribution<bool>>> distrs({ std::make_shared<const GenericDistributionBool>(p_death) });
	MortalityEnforcer me(PredicateFactory::make_true<Person>(), distrs);
	const Date asof(2012, 1, 1);
	Contexts ctx(asof);
	std::vector<std::shared_ptr<Person>> persons;
	persons.reserve(n_alive + n_dead);
	for (size_t i = 0; i < n_alive; ++i) {
		persons.push_back(std::make_shared<Person>(i + 1, PersonAttributes(Sex::FEMALE, 0), Date(1980, 1, 1)));
	}
	for (size_t i = 0; i < n_dead; ++i) {
		persons.push_back(std::make_shared<Person>(n_alive + i + 1, PersonAttributes(Sex::FEMALE, 0), Date(1920, 1, 1)));
		persons.back()->die(Date(2010, 1, 1));
	}
	me.apply(persons, ctx);
	size_t n_dead_after = 0;
	for (const auto& ptr : persons) {
		if (!ptr->is_alive(asof)) {
			++n_dead_after;
		}
	}
	ASSERT_EQ(25, n_dead_after);
}

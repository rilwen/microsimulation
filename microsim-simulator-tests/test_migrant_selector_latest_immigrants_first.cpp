#include <gtest/gtest.h>
#include "microsim-simulator/migration/migrant_selector_latest_immigrants_first.hpp"
#include "microsim-simulator/person.hpp"
#include "microsim-simulator/contexts.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(MigrantSelectLatestImmigrantsFirst, Test) {
	Contexts ctx;	
	std::vector<std::shared_ptr<Person>> result;
	auto p1 = std::make_shared<Person>(1, PersonAttributes(Sex::MALE, 0), Date(2001, 3, 23));
	auto p2 = std::make_shared<Person>(2, PersonAttributes(Sex::FEMALE, 0), Date(1981, 7, 1));
	p2->set_immigration_date(Date(2016, 5, 1));
	auto p3 = std::make_shared<Person>(3, PersonAttributes(Sex::FEMALE, 1), Date(1982, 7, 1));
	p3->set_immigration_date(Date(2014, 5, 1));
	std::vector<std::shared_ptr<Person>> source({ p1, p2, p3 });
	MigrantSelectorLatestImigrantsFirst selector;
	selector.select(ctx, source, result, 2);
	ASSERT_EQ(2, result.size());
	ASSERT_EQ(p2, result[0]);
	ASSERT_EQ(p3, result[1]);
	selector.select(ctx, source, result, 1);
	ASSERT_EQ(3, result.size());
	ASSERT_EQ(p2, result[0]);
	ASSERT_EQ(p3, result[1]);
	ASSERT_EQ(p2, result[2]);	
}

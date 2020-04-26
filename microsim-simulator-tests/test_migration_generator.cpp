// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/migration_generator.hpp"
#include "microsim-simulator/person.hpp"
#include "microsim-simulator/person_data.hpp"
#include "microsim-core/person_attributes.hpp"
#include "core/dates.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(MigrationGenerator, comigrate_children) {
	std::vector<std::shared_ptr<Person>> adults;
	const unsigned int comigr_age_limit = 10;
	const Date migration_date(2010, 1, 1);
	adults.push_back(std::make_shared<Person>(1, PersonAttributes(Sex::FEMALE, 0), Date(1983, 4, 5)));
	adults.push_back(std::make_shared<Person>(2, PersonAttributes(Sex::MALE, 0), Date(1983, 4, 5)));
	adults.push_back(std::make_shared<Person>(3, PersonAttributes(Sex::FEMALE, 0), Date(1982, 2, 15)));
	adults.push_back(std::make_shared<Person>(4, PersonAttributes(Sex::FEMALE, 0), Date(1965, 3, 12)));
	adults.push_back(std::make_shared<Person>(5, PersonAttributes(Sex::FEMALE, 0), Date(1931, 3, 12)));
	const auto child1 = std::make_shared<Person>(6, PersonAttributes(Sex::MALE, 0), Date(2010, 1, 5)); // will be comigrated
	Person::link_parents_child(child1, adults[0], Date(2009, 3, 2));
	const auto child2 = std::make_shared<Person>(7, PersonAttributes(Sex::FEMALE, 0), Date(2009, 9, 4)); // will be comigrated
	Person::link_parents_child(child2, adults[2], Date(2009, 1, 2));
	const auto child3 = std::make_shared<Person>(8, PersonAttributes(Sex::FEMALE, 0), Date(1991, 5, 20));  // will not be comigrated
	Person::link_parents_child(child3, adults[3], Date(1990, 8, 23));
	adults[4]->add_childbirth(Date(1952, 8, 13));
	const std::vector<std::shared_ptr<Person>> copy(adults);
	MigrationGenerator::comigrate_children(adults, migration_date, comigr_age_limit, 0);
	ASSERT_EQ(7, adults.size());
	ASSERT_EQ(child1, adults[5]);
	ASSERT_EQ(child2, adults[6]);
	adults = copy;
	MigrationGenerator::comigrate_children(adults, migration_date, comigr_age_limit, 1);
	ASSERT_EQ(6, adults.size());
	ASSERT_EQ(child2, adults[5]);
}

TEST(MigrationGenerator, convert_added_persons_to_data) {
	std::vector<std::shared_ptr<Person>> persons_added_ptrs;
	persons_added_ptrs.push_back(std::make_shared<Person>(1, PersonAttributes(Sex::FEMALE, 0), Date(1983, 4, 5)));
	persons_added_ptrs.push_back(std::make_shared<Person>(2, PersonAttributes(Sex::MALE, 0), Date(1983, 4, 5)));
	persons_added_ptrs.push_back(std::make_shared<Person>(3, PersonAttributes(Sex::FEMALE, 0), Date(1982, 2, 15)));
	persons_added_ptrs.push_back(std::make_shared<Person>(4, PersonAttributes(Sex::FEMALE, 0), Date(1965, 3, 12)));
	persons_added_ptrs.push_back(std::make_shared<Person>(5, PersonAttributes(Sex::FEMALE, 0), Date(1931, 3, 12)));
	persons_added_ptrs.push_back(std::make_shared<Person>(6, PersonAttributes(Sex::MALE, 0), Date(2010, 1, 5)));
	Person::link_parents_child(persons_added_ptrs[5], persons_added_ptrs[0], Date(2009, 3, 2));
	persons_added_ptrs.push_back(persons_added_ptrs[0]);
	persons_added_ptrs.push_back(persons_added_ptrs[5]); // duplicate mother and child
	std::vector<std::shared_ptr<Person>> copy(persons_added_ptrs);
	Contexts ctx(Date(2010, 1, 1));
	const size_t max_id = 100;
	ctx.mutable_ctx().increase_id(max_id);
	std::vector<PersonData> data;
	const Date migr_date(2010, 2, 15);
	MigrationGenerator::convert_added_persons_to_data(ctx, persons_added_ptrs, 0, data, migr_date);
	ASSERT_EQ(persons_added_ptrs.size(), data.size());
	auto it = copy.begin();
	Population::sort_persons(copy);
	std::set<Actor::id_t> new_ids;
	for (const auto& pd : data) {
		new_ids.insert(pd.id);
		ASSERT_NE(pd.id, (*it)->id()) << pd;
		ASSERT_GT(pd.id, max_id) << pd;
		if (pd.mother_id != Actor::INVALID_ID) {
			ASSERT_GT(pd.mother_id, max_id) << pd;
		}
		ASSERT_EQ((*it)->date_of_birth(), pd.date_of_birth) << pd;
		ASSERT_EQ((*it)->attributes(), pd.attributes) << pd;
		ASSERT_EQ(migr_date, pd.immigration_date) << pd;
		++it;
	}
	ASSERT_EQ(new_ids.size(), data.size());
}

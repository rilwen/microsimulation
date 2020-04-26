// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/population.hpp"
#include "microsim-simulator/history.hpp"
#include "microsim-simulator/person.hpp"

using namespace averisera;
using namespace averisera::microsim;


TEST(Population, Members) {
    Population pop;
    ASSERT_EQ(&pop.persons(), &pop.members<Person>());
}

TEST(Population, AddPerson) {
    Population pop;
    ASSERT_TRUE(pop.persons().empty());
    PersonAttributes pa(Sex::MALE, 1);
    std::shared_ptr<Person> p = std::make_shared<Person>(101, pa, Date(1989, 6, 4));
    pop.add_person(p);
    ASSERT_EQ(1u, pop.persons().size());
    ASSERT_EQ(p, pop.persons()[0]);
    p = std::make_shared<Person>(102, pa, Date(1989, 6, 4));
    pop.add_person(p);
    ASSERT_EQ(2u, pop.persons().size());
    ASSERT_EQ(p, pop.persons()[1]);
    p = std::make_shared<Person>(102, pa, Date(1989, 6, 4));
    ASSERT_THROW(pop.add_person(p), std::domain_error);
    ASSERT_THROW(pop.add_person(nullptr), std::domain_error);
}

TEST(Population, GetPerson) {
    Population pop;
    ASSERT_EQ(nullptr, pop.get_person(10));
    PersonAttributes pa(Sex::MALE, 1);
    std::shared_ptr<Person> p = std::make_shared<Person>(101, pa, Date(1989, 6, 4));
    pop.add_person(p);
    ASSERT_EQ(p, pop.get_person(101));
    ASSERT_EQ(nullptr, pop.get_person(102));
}

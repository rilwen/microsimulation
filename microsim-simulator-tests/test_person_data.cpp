// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/mutable_context.hpp"
#include "microsim-simulator/person_data.hpp"
#include <sstream>

using namespace averisera;
using namespace averisera::microsim;

TEST(PersonData, DefaultConstructor) {
	PersonData p;
	ASSERT_EQ(Actor::INVALID_ID, p.id);
	ASSERT_TRUE(p.histories.empty());
	ASSERT_EQ(Actor::INVALID_ID, p.mother_id);
	ASSERT_TRUE(p.children.empty());
	ASSERT_TRUE(p.childbirths.empty());
	ASSERT_TRUE(p.date_of_birth.is_not_a_date());
	ASSERT_TRUE(p.conception_date.is_not_a_date());
	ASSERT_TRUE(p.date_of_death.is_not_a_date());
}

TEST(PersonData, ResetIds) {
	std::vector<PersonData> p(4);
	p[0].id = 10;
	p[1].id = 20;
	p[2].id = 21;
	p[3].id = 30;

	p[1].mother_id = 10;
	p[2].mother_id = 10;

	p[3].mother_id = 20;

	p[0].children.resize(2);
	p[0].children[0] = 21;
	p[0].children[1] = 20;

	p[1].children.resize(1);
	p[1].children[0] = 30;

	MutableContext ctx;
	PersonData::reset_ids(p, ctx);
	for (size_t i = 0; i < 4u; ++i) {
		ASSERT_EQ(i + 1, p[i].id) << i;
	}

	ASSERT_EQ(1u, p[1].mother_id);
	ASSERT_EQ(1u, p[2].mother_id);
	ASSERT_EQ(2u, p[3].mother_id);
	ASSERT_EQ(3u, p[0].children[0]);
	ASSERT_EQ(2u, p[0].children[1]);
	ASSERT_EQ(4u, p[1].children[0]);
}

TEST(PersonData, ChangeChildId) {
	PersonData p;
	p.id = 1;
	p.children.push_back(2);
	ASSERT_THROW(p.change_child_id(30, 3), std::out_of_range);
	p.change_child_id(2, 3);
	ASSERT_EQ(3u, p.children[0]);
}

TEST(PersonData, Print) {
	PersonData p;
	p.id = 2;
	p.date_of_birth = Date(1981, 12, 13);
	p.date_of_death = Date(2017, 5, 1);
	p.fetuses.push_back(Fetus(PersonAttributes(Sex::MALE, 0), Date(2012, 1, 1)));
	std::stringstream ss;
	p.print(ss);
	ASSERT_EQ("ID=2\nHISTORIES=[]\nATTRIBS=(FEMALE, 0)\nDOB=1981-12-13\nDOD=2017-05-01\nCONCEPTION_DATE=not-a-date-time\nIMMIGRATION_DATE=not-a-date-time\nMOTHER_ID=0\nCHILDREN=[]\nCHILDBIRTHS=[]\nFETUSES=[((MALE, 0), 2012-01-01)]\n", ss.str());
}

TEST(PersonData, LinkChild) {
	PersonData mother;
	mother.id = 2;
	PersonData child;
	child.date_of_birth = Date(2000, 9, 1);
	child.id = 3;
	Date conception_date(2000, 1, 1);
	mother.link_child(child, conception_date);
	ASSERT_EQ(child.mother_id, mother.id);
	ASSERT_EQ(1u, mother.children.size());
	ASSERT_EQ(conception_date, child.conception_date);
	ASSERT_EQ(child.id, mother.children[0]);
}

/*
 * ( *C) Averisera Ltd 2015
 */
#include <gtest/gtest.h>
#include "core/daycount.hpp"
#include "core/period.hpp"
#include "microsim-simulator/person.hpp"
#include "microsim-simulator/person_data.hpp"
#include "microsim-simulator/history_factory.hpp"
#include "microsim-simulator/immutable_context.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(Person, Constructor) {
    PersonAttributes pa(Sex::MALE, 1);
    const Date dob(1989, 6, 4);
    Person p(101, pa, dob);
	ASSERT_EQ(1989, p.year_of_birth());
    ASSERT_EQ(pa, p.attributes());
	ASSERT_EQ(pa.sex(), p.sex());
	ASSERT_EQ(pa.ethnicity(), p.ethnicity());
    ASSERT_EQ(101u, p.id());
    ASSERT_EQ(dob, p.date_of_birth());
    ASSERT_FALSE(p.died());
    ASSERT_TRUE(p.is_alive(dob));
    ASSERT_FALSE(p.is_alive(dob - Period::days(1)));
    //std::cout << sizeof(p) << std::endl;
}

TEST(Person, Age) {
    Person p(10, PersonAttributes(Sex::FEMALE, 0), Date(1989, 6, 4));
    ASSERT_EQ(29u, p.age(Date(2019, 5, 5)));
	ASSERT_EQ(0u, p.age(Date(1988, 1, 1)));
	ASSERT_EQ(0u, p.age(Date(1989, 6, 7)));
	ASSERT_EQ(29u, p.age(Date(2019, 6, 3)));
	ASSERT_EQ(30u, p.age(Date(2019, 6, 4)));
	ASSERT_EQ(30u, p.age(Date(2019, 6, 5)));

	Person p2(11, PersonAttributes(Sex::MALE, 1), Date(1992, 2, 29));
	ASSERT_EQ(1, p2.age(Date(1993, 2, 28)));
	ASSERT_EQ(3, p2.age(Date(1996, 2, 28)));
	ASSERT_EQ(4, p2.age(Date(1996, 2, 29)));
}

TEST(Person, AgeFract) {
    const Date dob(1989, 6, 4);
    Person p(10, PersonAttributes(Sex::FEMALE, 0), dob);
    const Date date(2019, 7, 4);
    const double expected_age_fract = Daycount::YEAR_FRACT()->calc(dob, date);
    ASSERT_NEAR(expected_age_fract, p.age_fract(date), 1E-10);
	ASSERT_EQ(0.0, p.age_fract(Date(1989, 6, 3)));
}

TEST(Person, IsAlive) {
    const Date dob = Date(1989, 6, 4);
    Person p(10, PersonAttributes(Sex::FEMALE, 0), dob);
    ASSERT_TRUE(p.is_alive(dob));
    ASSERT_TRUE(p.is_alive(Date(2011, 5, 3)));
    ASSERT_FALSE(p.is_alive(Date(1989, 6, 3)));
    p.die(Date(2020, 12, 25));
    ASSERT_TRUE(p.is_alive(Date(2011, 5, 3)));
    ASSERT_TRUE(p.is_alive(Date(2020, 12, 24))) << "I'm not dead yet!";
    ASSERT_FALSE(p.is_alive(Date(2020, 12, 25)));
    ASSERT_FALSE(p.is_alive(Date(2021, 12, 25)));
}

TEST(Person, Die) {
    Person p(101, PersonAttributes(Sex::FEMALE, 2), Date(1989, 6, 4));
    Person& p2 = p.die(Date(2003, 4, 1));
    ASSERT_EQ(&p2, &p);
    ASSERT_TRUE(p.died());
    ASSERT_TRUE(p.is_alive(Date(2003,3,31)));
    ASSERT_FALSE(p.is_alive(Date(2003,4,1)));
    ASSERT_FALSE(p.is_alive(Date(2003,4,2)));
}

TEST(Person, Histories) {
    std::vector<std::unique_ptr<History>> histories;
    histories.push_back(std::move(HistoryFactory::DENSE<double>()("")));
    Person p(101, PersonAttributes(Sex::FEMALE, 2), Date(1989, 6, 4));
    p.set_histories(std::move(histories));
    ASSERT_TRUE(p.history(0).empty());
    histories.resize(0);
    histories.push_back(std::move(HistoryFactory::DENSE<double>()("")));
    ASSERT_THROW(p.set_histories(std::move(histories)), std::logic_error);
}

TEST(Person, ToData) {
	Person p(101, PersonAttributes(Sex::FEMALE, 2), Date(1989, 6, 4));
	const Fetus fetus(PersonAttributes(Sex::MALE, 2), Date(2015, 7, 1));
	p.add_fetus(fetus);
	p.set_immigration_date(Date(2008, 12, 4));
	p.add_childbirth(Date(2011, 4, 20));
	ImmutableContext im_ctx;
	PersonData data(p.to_data(im_ctx));
	data.conception_date = Date(1988, 9, 1);
	auto p2 = Person::from_data(std::move(data), im_ctx, false);
	ASSERT_EQ(1, p2->nbr_fetuses());
	ASSERT_EQ(p.date_of_birth(), p2->date_of_birth());
	ASSERT_EQ(p.attributes(), p2->attributes());
	ASSERT_EQ(p.immigration_date(), p2->immigration_date());
	ASSERT_EQ(p.nbr_children(), p2->nbr_children());
	ASSERT_EQ(data.conception_date, p2->conception_date());
}

TEST(Person, ToDataMotherAndChild) {
	Contexts ctx(Date(2020, 9, 1));
	std::shared_ptr<Person> mother_ptr(new Person(ctx.mutable_ctx().gen_id(), PersonAttributes(Sex::FEMALE, 2), Date(2000, 1, 1)));
	const Fetus fetus(PersonAttributes(Sex::MALE, 2), Date(2020, 1, 1));
	mother_ptr->add_fetus(fetus);
	mother_ptr->give_birth(Date(2020, 9, 1), ctx, mother_ptr);
	const auto child_ptr = mother_ptr->get_child(0);
	ASSERT_GT(child_ptr->id(), mother_ptr->id());
	ASSERT_EQ(mother_ptr->id(), child_ptr->mother().lock()->id());
	PersonData child_data(child_ptr->to_data(ctx.immutable_ctx()));
	ASSERT_EQ(mother_ptr->id(), child_data.mother_id);
	ASSERT_EQ(child_ptr->conception_date(), child_data.conception_date);
	PersonData mother_data(mother_ptr->to_data(ctx.immutable_ctx()));
	ASSERT_EQ(1u, mother_data.children.size());
	ASSERT_EQ(child_ptr->id(), mother_data.children[0]);
}

TEST(Person, Fetuses) {
	Person p(101, PersonAttributes(Sex::FEMALE, 2), Date(1989, 6, 4));
	ASSERT_EQ(0, p.nbr_fetuses());
	p.add_fetus(Fetus(PersonAttributes(Sex::MALE, 2), Date(2009, 1, 1)));
	ASSERT_EQ(1, p.nbr_fetuses());
	p.add_fetus(Fetus(PersonAttributes(Sex::FEMALE, 2), Date(2010, 10, 1)));
	ASSERT_EQ(2, p.nbr_fetuses());
	p.remove_fetuses(Date(2010, 2, 15));
	ASSERT_EQ(1, p.nbr_fetuses());
	p.remove_fetuses(Date(2011, 7, 1));
	ASSERT_EQ(0, p.nbr_fetuses());
	p.add_fetus(Fetus(PersonAttributes(Sex::FEMALE, 2), Date(2013, 10, 1)));
	ASSERT_EQ(1, p.nbr_fetuses());
	ASSERT_THROW(p.add_fetus(Fetus(PersonAttributes(Sex::FEMALE, 2), Date(2013, 9, 11))), std::logic_error);
	Person p2(101, PersonAttributes(Sex::MALE, 2), Date(1989, 6, 4));
	ASSERT_EQ(0, p2.nbr_fetuses());
	ASSERT_NO_THROW(p2.remove_fetuses(Date(2010, 1, 1)));
}

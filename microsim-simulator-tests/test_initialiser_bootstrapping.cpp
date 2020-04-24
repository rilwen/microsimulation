#include <gtest/gtest.h>
#include "microsim-simulator/contexts.hpp"
#include "microsim-simulator/immutable_context.hpp"
#include "microsim-simulator/initialiser/initialiser_bootstrapping_unlinked.hpp"
#include "microsim-simulator/initialiser/initialiser_bootstrapping_with_links.hpp"
#include "microsim-simulator/mutable_context.hpp"
#include "microsim-simulator/person_data.hpp"
#include "microsim-simulator/population_data.hpp"
#include "helpers.hpp"

using namespace averisera;
using namespace averisera::microsim;
//using namespace averisera::testing;

static std::vector<PersonData> make_population() {
    // mother and child
    std::vector<PersonData> sample;
    PersonData pd;
    pd.id = 1;
    pd.attributes = PersonAttributes(Sex::FEMALE, 0);
    pd.date_of_birth = Date(1956, 4, 20);
    sample.push_back(std::move(pd));

    pd = PersonData();
    pd.id = 2;
    pd.attributes = PersonAttributes(Sex::MALE, 0);
    pd.date_of_birth = Date(1977, 10, 12);
    sample.push_back(std::move(pd));

    pd = PersonData();
    pd.id = 3;
    pd.attributes = PersonAttributes(Sex::FEMALE, 0);
    pd.date_of_birth = Date(1980, 2, 7);
    sample.push_back(std::move(pd));

    sample[0].link_child(sample[1], Date(1977, 1, 13));
    sample[0].link_child(sample[2], Date(1979, 5, 3));

    return sample;
}

TEST(InitialiserBootstrapping, Unlinked) {
    Contexts ctx(ctx_with_rng_precalc({0.0, 0.5}));
    //Contexts ctx(std::make_shared<ImmutableContext>(), std::make_shared<MutableContext>(std::unique_ptr<RNG>(new RNGPrecalc({0.0, 0.5}))));
    InitialiserBootstrappingUnlinked initialiser(std::make_unique<InitialiserBootstrapping::PersonDataSamplerFromData>(make_population()));
    PopulationData result(initialiser.initialise(2u, ctx));
	const auto sample = make_population();
    ASSERT_EQ(2u, result.persons.size());
    ASSERT_EQ(sample[0].date_of_birth, result.persons[0].date_of_birth);
    ASSERT_EQ(0, result.persons[0].children.size());
    ASSERT_TRUE(result.persons[0].mother_id == Actor::INVALID_ID);
    ASSERT_EQ(sample[1].date_of_birth, result.persons[1].date_of_birth);
    ASSERT_EQ(0, result.persons[1].children.size());
    ASSERT_TRUE(result.persons[1].mother_id == Actor::INVALID_ID);
}

TEST(InitialiserBootstrapping, WithLinks) {
    Contexts ctx(ctx_with_rng_precalc({0.5, 1.0}));
    //Contexts ctx(std::make_shared<ImmutableContext>(), std::make_shared<MutableContext>(std::unique_ptr<RNG>(new RNGPrecalc({0.5, 1.0}))));
    InitialiserBootstrappingWithLinks initialiser(std::make_unique<InitialiserBootstrapping::PersonDataSamplerFromData>(make_population()), 1);
    PopulationData result(initialiser.initialise(4u, ctx));
	const auto sample = make_population();
    ASSERT_THROW(ctx.mutable_ctx().rng().next_uniform(), std::runtime_error) << "RNG should be exhausted";
    const auto& persons = result.persons;
    ASSERT_EQ(4u, persons.size());
    ASSERT_EQ(sample[1].date_of_birth, persons[0].date_of_birth);
    ASSERT_EQ(sample[0].date_of_birth, persons[1].date_of_birth);
    ASSERT_EQ(sample[2].date_of_birth, persons[2].date_of_birth);
    ASSERT_EQ(sample[2].date_of_birth, persons[3].date_of_birth);
    ASSERT_EQ(0, persons[3].children.size());
    ASSERT_TRUE(persons[3].mother_id == Actor::INVALID_ID);
	ASSERT_EQ(persons[1].id, persons[0].mother_id);
	ASSERT_EQ(persons[1].id, persons[2].mother_id);
    ASSERT_EQ(2u, persons[1].children.size());
    ASSERT_EQ(persons[0].id, persons[1].children[0]);
    ASSERT_EQ(persons[2].id, persons[1].children[1]);
}

TEST(InitialiserBootstrapping, WithLinksZeroDepth) {
    Contexts ctx(ctx_with_rng_precalc({0.0, 0.5}));
    //Contexts ctx(std::make_shared<ImmutableContext>(), std::make_shared<MutableContext>(std::unique_ptr<RNG>(new RNGPrecalc({ 0.0, 0.5 }))));
    InitialiserBootstrappingWithLinks initialiser(std::make_unique<InitialiserBootstrapping::PersonDataSamplerFromData>(make_population()), 0);
    PopulationData result(initialiser.initialise(2u, ctx));
    const auto sample = make_population();
    ASSERT_EQ(2u, result.persons.size());
    ASSERT_EQ(sample[0].date_of_birth, result.persons[0].date_of_birth);
    ASSERT_EQ(0, result.persons[0].children.size());
    ASSERT_TRUE(result.persons[0].mother_id == Actor::INVALID_ID);
    ASSERT_EQ(sample[1].date_of_birth, result.persons[1].date_of_birth);
    ASSERT_EQ(0, result.persons[1].children.size());
    ASSERT_TRUE(result.persons[1].mother_id == Actor::INVALID_ID);

}

TEST(InitialiserBootstrapping, PersonDataSamplerFromData) {
	std::vector<PersonData> data(3);
	data[0].id = 10;
	data[0].date_of_birth = Date(1939, 9, 1);
	data[1].id = 20;
	data[1].date_of_birth = Date(1989, 6, 4);
	data[2].id = 21;
	data[2].date_of_birth = Date(1981, 12, 13);
	InitialiserBootstrapping::PersonDataSamplerFromData sampler(std::move(data));
	ASSERT_EQ(3, sampler.sample_size());
	ASSERT_EQ(20, sampler.sample_person(1).id);
	ASSERT_EQ(2, sampler.find_by_id(21));
	ASSERT_THROW(sampler.find_by_id(100), std::out_of_range);
}

TEST(InitialiserBootstrapping, PersonDataSamplerFromPersons) {
	Contexts ctx;
	std::vector<std::shared_ptr<Person>> persons(3);
	persons[0] = std::make_shared<Person>(10, PersonAttributes(Sex::FEMALE, 0), Date(1981, 5, 1));
	persons[1] = std::make_shared<Person>(20, PersonAttributes(Sex::MALE, 0), Date(1981, 12, 13));
	persons[2] = std::make_shared<Person>(21, PersonAttributes(Sex::MALE, 1), Date(1989, 6, 4));
	InitialiserBootstrapping::PersonDataSamplerFromPersons sampler(persons, ctx.immutable_ctx());
	ASSERT_EQ(3, sampler.sample_size());
	ASSERT_EQ(2, sampler.find_by_id(21));
	ASSERT_THROW(sampler.find_by_id(100), std::out_of_range);
	const PersonData& pd1 = sampler.sample_person(0);
	ASSERT_EQ(persons[0]->id(), pd1.id);
	ASSERT_EQ(persons[0]->date_of_birth(), pd1.date_of_birth);
	ASSERT_EQ(persons[0]->attributes(), pd1.attributes);
	ASSERT_EQ(&pd1, &sampler.sample_person(0));
}

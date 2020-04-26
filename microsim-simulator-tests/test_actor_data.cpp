// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/actor_data.hpp"
#include "microsim-simulator/mutable_context.hpp"
#include "microsim-simulator/history_factory.hpp"
#include <sstream>

using namespace averisera;
using namespace averisera::microsim;

TEST(ActorData, SortByIdAndSwapAndFindById) {
    std::vector<ActorData> actors;
    ActorData a;
    a.id = 20;
    actors.push_back(std::move(a));
    a.id = 5;
    actors.push_back(std::move(a));
    ActorData::sort_by_id(actors);
    ASSERT_EQ(2u, actors.size());
    ASSERT_EQ(5, actors[0].id);
    ASSERT_EQ(20, actors[1].id);

    actors[0].swap(actors[1]);
    ASSERT_EQ(5, actors[1].id);
    ASSERT_EQ(20, actors[0].id);

    actors[0].swap(actors[1]);
    ASSERT_EQ(actors.end(), ActorData::find_by_id(actors, 7));
    ASSERT_EQ(5, ActorData::find_by_id(actors, 5)->id);
}

TEST(ActorData, DefaultConstructor) {
	ActorData a;
	ASSERT_EQ(Actor::INVALID_ID, a.id);
	ASSERT_TRUE(a.histories.empty());
}

TEST(ActorData, MoveConstructor) {
	ActorData a1;
	a1.id = 100;
	a1.histories.insert(std::make_pair("BMI", HistoryData("BMI", ObjectVector::Type::DOUBLE)));
	const std::string* nameptr = &a1.histories.find("BMI")->first;
	ActorData a2(std::move(a1));
	ASSERT_EQ(100, a2.id);
	ASSERT_EQ(1u, a2.histories.size());
	ASSERT_EQ(nameptr, &a2.histories.find("BMI")->first);
	ASSERT_TRUE(a1.histories.empty());
}

TEST(ActorData, Clone) {
	ActorData a1;
	a1.id = 100;
	a1.histories.insert(std::make_pair("BMI", HistoryData("double", "BMI")));
	a1.histories["BMI"].append(Date(2012, 4, 4), 0.2);
	const std::string* nameptr = &a1.histories.find("BMI")->first;
	ActorData a2(a1.clone());
	ASSERT_EQ(100, a2.id);
	ASSERT_EQ(1u, a2.histories.size());
	ASSERT_NE(nameptr, &a2.histories.find("BMI")->first);
	ASSERT_FALSE(a1.histories.empty());
	ASSERT_EQ(1u, a1.histories["BMI"].size());
	ASSERT_EQ(a1.histories["BMI"].size(), a2.histories["BMI"].size());
}

TEST(ActorData, GetHistory) {
	ActorData a1;
	a1.id = 100;
	a1.histories.insert(std::make_pair("BMI", HistoryData("double", "BMI")));
	ASSERT_NE(a1.get_history("BMI"), a1.histories.end());
	ASSERT_EQ(a1.get_history("Chlamydia"), a1.histories.end());
}

TEST(ActorData, Print) {
	ActorData a1;
	a1.id = 100;
	a1.histories.insert(std::make_pair("BMI", HistoryData("sparse double", "BMI")));
	a1.histories["BMI"].append(Date(2012, 4, 4), 0.2);
	std::stringstream ss;
	a1.print(ss);
	ASSERT_EQ("ID=100\nHISTORIES=[<BMI, {NAME=BMI|DATES=[2012-04-04]|VALUES=double|[0.2]|FACTORY_TYPE=sparse double}>]\n", ss.str());
}

TEST(ActorData, CleanupOriginalIds) {
	std::vector<ActorData> actors(3);
	actors[0].id = Actor::INVALID_ID;
	actors[1].id = 5;
	actors[2].id = 100;
	MutableContext mctx;
	mctx.increase_id(20);
	ASSERT_THROW(ActorData::cleanup_original_ids(actors, mctx), std::runtime_error);
	actors[0].id = Actor::INVALID_ID;
	actors[1].id = 21;
	actors[2].id = 100;
	ActorData::cleanup_original_ids(actors, mctx);
	ASSERT_EQ(101u, actors[0].id);
	ASSERT_EQ(21u, actors[1].id);
	ASSERT_EQ(100u, actors[2].id);
	ASSERT_EQ(101u, mctx.get_max_id());
}

TEST(ActorData, ResetIds) {
	std::vector<ActorData> actors(3);
	actors[0].id = Actor::INVALID_ID;
	actors[1].id = 5;
	actors[2].id = 100;
	MutableContext mctx;
	ActorData::id_map_t old_to_new;
	ActorData::reset_ids(actors, mctx, old_to_new);
	ASSERT_EQ(2u, old_to_new.size());
	ASSERT_EQ(1, actors[0].id);
	ASSERT_EQ(2, actors[1].id);
	ASSERT_EQ(3, actors[2].id);
	ASSERT_EQ(3, mctx.get_max_id());
}

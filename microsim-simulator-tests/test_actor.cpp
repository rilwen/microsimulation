// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/actor.hpp"
#include "microsim-simulator/actor_data.hpp"
#include "microsim-simulator/history.hpp"
#include "microsim-simulator/immutable_context.hpp"

using namespace averisera::microsim;

TEST(Actor, Constants) {
    ASSERT_EQ(0u, Actor::INVALID_ID);
    ASSERT_EQ(1u, Actor::MIN_ID);
}

class MockActor : public Actor {
public:
	MockActor(Actor::id_t id)
		: Actor(id) {}

	const HistoryRegistry& get_history_registry(const ImmutableContext& im_ctx) const override {
		return im_ctx.person_history_registry();
	}
};

TEST(Actor, ToDataNullHistory) {
	MockActor actor(1);
	std::vector<std::unique_ptr<History>> histories;
	histories.push_back(nullptr);
	actor.set_histories(std::move(histories));
	ImmutableContext imm_ctx;
	imm_ctx.register_person_variable("A", HistoryFactory::DENSE<double>());
	ActorData data(actor.to_data(imm_ctx));
	ASSERT_EQ(actor.id(), data.id);
	ASSERT_EQ(0, data.histories.size());
}

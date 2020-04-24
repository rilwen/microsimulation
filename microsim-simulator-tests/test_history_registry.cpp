/*
* (C) Averisera Ltd 2015
*/
#include <gtest/gtest.h>
#include "microsim-simulator/history_registry.hpp"
#include "microsim-simulator/history.hpp"

using namespace averisera;
using namespace averisera::microsim;

// static int _factory_flag = -1;

// std::unique_ptr<History> factory0() {
// 	_factory_flag = 0;
// 	return nullptr;
// }

// std::unique_ptr<History> factory1() {
// 	_factory_flag = 1;
// 	return nullptr;
// }

TEST(HistoryRegistry, Test) {
	HistoryRegistry registry;
	ASSERT_EQ(0u, registry.nbr_variables());
	ASSERT_FALSE(registry.has_variable("BMI"));
	ASSERT_THROW(registry.variable_index("BMI"), std::domain_error);
	const auto idx0 = registry.register_variable("BMI");
	ASSERT_EQ(0u, idx0);
	ASSERT_EQ(idx0, registry.variable_index("BMI"));
	ASSERT_EQ(1u, registry.nbr_variables());
	ASSERT_TRUE(registry.has_variable("BMI"));
	//registry.variable_history_factory(0)();
	//ASSERT_EQ(0, _factory_flag);
	ASSERT_EQ("BMI", registry.variable_name(0));
	const auto idx1 = registry.register_variable("WHVAL");
	ASSERT_EQ(1u, idx1);
    ASSERT_THROW(registry.register_variable("BMI"), std::domain_error);
	ASSERT_EQ(idx1, registry.variable_index("WHVAL"));
	ASSERT_EQ(2u, registry.nbr_variables());
	ASSERT_TRUE(registry.has_variable("WHVAL"));
	//registry.variable_history_factory(1)();
	//ASSERT_EQ(1, _factory_flag);
	ASSERT_EQ("WHVAL", registry.variable_name(1));

    ASSERT_THROW(registry.register_variable(""), std::domain_error) << "We should not be able to register an empty name";
}

/*
 * (C) Averisera Ltd 2015
 */
#include <gtest/gtest.h>
#include "core/object_value.hpp"
#include "microsim-core/config_interpreter.hpp"
#include <utility>

using namespace averisera;
using namespace averisera::microsim;

TEST(ConfigInterpreter, TrimWhitespace) {
    const std::string input("\tAla ma\tkota \r\n");
    ASSERT_EQ("Ala ma\tkota", ConfigInterpreter::trim_whitespace(input));
}

TEST(ConfigInterpreter, FindAtLevel) {
    ASSERT_EQ(-1, ConfigInterpreter::find_at_level("eval(1+2)", ',', 0));
    ASSERT_EQ(-1, ConfigInterpreter::find_at_level("eval(1+2)", '+', 0));
    ASSERT_EQ(6, ConfigInterpreter::find_at_level("eval(1+2)", '+', 1));
    ASSERT_EQ(4, ConfigInterpreter::find_at_level("eval(1+2)", '(', 0));
    ASSERT_EQ(8, ConfigInterpreter::find_at_level("eval(1+2)", ')', 0));
    ASSERT_THROW(ConfigInterpreter::find_at_level("eval((1+2)", '+', 0), std::runtime_error);
    ASSERT_THROW(ConfigInterpreter::find_at_level("eval(1+2))", '+', 0), std::runtime_error);
    ASSERT_THROW(ConfigInterpreter::find_at_level("eval(1+2", '+', 0), std::runtime_error);
    ASSERT_THROW(ConfigInterpreter::find_at_level("eval)1+2)", '+', 0), std::runtime_error);
}

TEST(ConfigInterpreter, SplitExpr) {
    std::pair<std::string, std::vector<std::string>> result;
    bool is_call;
    ASSERT_THROW(ConfigInterpreter::split_expr("", is_call), std::runtime_error);
    result = ConfigInterpreter::split_expr("constant", is_call);
    ASSERT_EQ("constant", result.first);
    ASSERT_TRUE(result.second.empty());
    ASSERT_FALSE(is_call);
    result = ConfigInterpreter::split_expr("func(arg)", is_call);
    ASSERT_EQ("func", result.first);
    ASSERT_EQ(std::vector<std::string>({ "arg" }), result.second);
    ASSERT_TRUE(is_call);
    result = ConfigInterpreter::split_expr("_func(arg1, arg2,arg3)", is_call);
    ASSERT_EQ("_func", result.first);
    ASSERT_EQ(std::vector<std::string>({ "arg1", "arg2", "arg3" }), result.second);
    ASSERT_TRUE(is_call);
    result = ConfigInterpreter::split_expr("func()", is_call);
    ASSERT_EQ("func", result.first);
    ASSERT_TRUE(result.second.empty());
    ASSERT_TRUE(is_call);
}

TEST(ConfigInterpreter, ParseToken) {
    ASSERT_EQ("", ConfigInterpreter::parse_token(" \"\"\r\n").as_string());
    ASSERT_EQ("foo", ConfigInterpreter::parse_token(" \"foo\"\n").as_string());
    ASSERT_EQ("2", ConfigInterpreter::parse_token("\"2\"").as_string());
    ASSERT_EQ("false", ConfigInterpreter::parse_token(" \"false\"\n").as_string());
    ASSERT_EQ("a\\=\"b\"", ConfigInterpreter::parse_token(" \"a\\\\=\\\"b\\\"\"\n").as_string());
    ASSERT_EQ(0.2, ConfigInterpreter::parse_token(" 0.2 ").as_double());
    ASSERT_EQ(-0.2, ConfigInterpreter::parse_token(" -0.2 ").as_double());
    ASSERT_TRUE(ConfigInterpreter::parse_token(" true\t").as_bool());
    ASSERT_FALSE(ConfigInterpreter::parse_token("\tfalse").as_bool());
    ASSERT_THROW(ConfigInterpreter::parse_token(""), std::runtime_error);
    ASSERT_THROW(ConfigInterpreter::parse_token("\\a"), std::runtime_error);
    ASSERT_THROW(ConfigInterpreter::parse_token("\"a"), std::runtime_error);
    ASSERT_THROW(ConfigInterpreter::parse_token("b\""), std::runtime_error);
    ASSERT_THROW(ConfigInterpreter::parse_token("\""), std::runtime_error);
    ASSERT_THROW(ConfigInterpreter::parse_token("\"a\\\""), std::runtime_error);
    ASSERT_THROW(ConfigInterpreter::parse_token("a1"), std::runtime_error);
    ASSERT_THROW(ConfigInterpreter::parse_token("2.1a"), std::runtime_error);
    ASSERT_THROW(ConfigInterpreter::parse_token("2.1.3"), std::runtime_error);
}

static ConfigInterpreter::value_type add_(const ConfigInterpreter::argvec_type& args) {
	return args.at(0).as_double() + args.at(1).as_double();
}

static ConfigInterpreter::value_type sub_(const ConfigInterpreter::argvec_type& args) {
	return args.at(0).as_double() - args.at(1).as_double();
}

static ConfigInterpreter::value_type mul_(const ConfigInterpreter::argvec_type& args) {
	return args.at(0).as_double() * args.at(1).as_double();
}

TEST(ConfigInterpreter, InterpretExpression) {
    ConfigInterpreter ci;
	ci.add_function("add", add_);
    ci.add_function("sub", sub_);
    ci.add_function("mul", mul_);
    ASSERT_EQ(9, ci.interpret_expression("add( 1, mul ( sub(10,6), +2.0))").as_double());
    ASSERT_THROW(ci.interpret_expression("add( 1, mul ( sub(10), +2.0))").as_double(), std::exception);
    ASSERT_THROW(ci.interpret_expression("foo(2, 3)"), std::runtime_error);
    ASSERT_THROW(ci.interpret_expression("ad d(2, 3)"), std::runtime_error);
    ASSERT_THROW(ci.interpret_expression("1add(2, 3)"), std::runtime_error);
    ASSERT_THROW(ci.add_function("2add", add_), std::runtime_error);
    ASSERT_THROW(ci.add_function("ad d", add_), std::runtime_error);
    
    typedef std::pair<double, bool> pair_t;
    ci.add_function("pair", [](const ConfigInterpreter::argvec_type& args){ return ObjectValue::from_object(std::make_shared<pair_t>(args.at(0).as_double(), args.at(1).as_bool())); });
    const ObjectValue result = ci.interpret_expression("pair(0.2, true)");
    ASSERT_EQ(ObjectValue::Type::OBJECT, result.type());
    ASSERT_EQ(0.2, result.as_object<pair_t>()->first);
    ASSERT_EQ(true, result.as_object<pair_t>()->second);
}

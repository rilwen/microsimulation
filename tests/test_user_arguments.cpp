/*
(C) Averisera Ltd 2014
*/
#include <gtest/gtest.h>
#include "core/user_arguments.hpp"
#include <sstream>

TEST(UserArguments, SplitList) {
	std::vector<std::string> elems;
	averisera::UserArguments::split_list("Ala,Ma,Kota", elems);
	ASSERT_EQ(3u, elems.size());
	ASSERT_EQ("Ala", elems[0]);
	ASSERT_EQ("Ma", elems[1]);
	ASSERT_EQ("Kota", elems[2]);

	averisera::UserArguments::split_list("Singleton", elems);
	ASSERT_EQ(1u, elems.size());
	ASSERT_EQ("Singleton", elems[0]);
}

TEST(UserArguments, GetKeyValue) {
	std::string key;
	std::string value;
	averisera::UserArguments::get_key_value("input=data.tab", key, value);
	ASSERT_EQ("input", key);
	ASSERT_EQ("data.tab", value);
	averisera::UserArguments::get_key_value("\tinput = data.tab  ", key, value);
	ASSERT_EQ("input", key);
	ASSERT_EQ("data.tab", value);
	averisera::UserArguments::get_key_value("input=\"new data.tab\"", key, value);
	ASSERT_EQ("input", key);
	ASSERT_EQ("new data.tab", value);
}

TEST(UserArguments, GetKeysValues) {
	std::stringstream ss;
	ss << "param1=value1\n";
	ss << " \t\n";
	ss << "param2 =value2\n";
	ss << " #sss\n";
	ss << "param3= value3\n";
	ss << "#\n";
	ss << "param4 = value4\t\n";
	ss << "param5 = \"value 5\"\n";
	ss << "\n";
	ss << "\"param6=value6";
	std::unordered_map<std::string, std::string> map = averisera::UserArguments::get_keys_values(ss);
	ASSERT_EQ(6u, map.size());
	ASSERT_EQ("value1", map["param1"]);
	ASSERT_EQ("value2", map["param2"]);
	ASSERT_EQ("value3", map["param3"]);
	ASSERT_EQ("value4", map["param4"]);
	ASSERT_EQ("value 5", map["param5"]);
	ASSERT_EQ("value6", map["\"param6"]);
}

TEST(UserArguments, Object) {
	std::stringstream ss;
	ss << "alpha = 0.95\n";	
	ss << "# comment\n";
	ss << "dim = 3\n";	
	ss << "label = Foo\n";
	ss << "switch = 1";
	averisera::UserArguments ua(ss);
	ASSERT_EQ(4u, ua.size());
	ASSERT_EQ("Foo", ua.get<std::string>("label"));
	ASSERT_EQ(0.95, ua.get("alpha", 0.));
	ASSERT_EQ(3u, ua.get<unsigned int>("dim"));	
	ASSERT_TRUE(true == ua.get<bool>("switch")); // hack
	ASSERT_THROW(ua.get<bool>(""), std::domain_error);
	ASSERT_THROW(ua.get("", 0.2), std::domain_error);

	ASSERT_TRUE(ua.contains("alpha"));
	ASSERT_FALSE(ua.contains("beta"));
	ASSERT_THROW(ua.contains(""), std::domain_error);
}

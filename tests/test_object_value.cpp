// (C) Averisera Ltd 2014-2020
#include <sstream>
#include <gtest/gtest.h>
#include "core/object_value.hpp"

class MockObject {
public:
	MockObject(double a)
		: _a(a)
	{}

	double a() const {
		return _a;
	}
private:
	double _a;
};

TEST(ObjectValue, Test) {
	averisera::ObjectValue ov_double(0.5);
	EXPECT_EQ(averisera::ObjectValue::Type::DOUBLE, ov_double.type());
	EXPECT_EQ(0.5, ov_double.as_double());

	averisera::ObjectValue ov_int(-1);
	EXPECT_EQ(averisera::ObjectValue::Type::INT, ov_int.type());
	EXPECT_EQ(-1, ov_int.as_int());

	averisera::ObjectValue ov_bool(false);
	EXPECT_EQ(averisera::ObjectValue::Type::BOOL, ov_bool.type());
	EXPECT_TRUE(false == ov_bool.as_bool()); // workaround for a bug in googletest (or gcc?)

	averisera::ObjectValue ov_string("FOO");
	EXPECT_EQ(averisera::ObjectValue::Type::STRING, ov_string.type());
	EXPECT_EQ("FOO", ov_string.as_string());

	ov_string = averisera::ObjectValue(std::string("BAR"));
	EXPECT_EQ(averisera::ObjectValue::Type::STRING, ov_string.type());
	EXPECT_EQ("BAR", ov_string.as_string());

	std::shared_ptr<MockObject> mo(new MockObject(2.1));
	averisera::ObjectValue ov_object(averisera::ObjectValue::from_object(mo));
	EXPECT_EQ(averisera::ObjectValue::Type::OBJECT, ov_object.type());
	EXPECT_EQ(2.1, ov_object.as_object<MockObject>()->a());
	EXPECT_NE(nullptr, ov_object.as_object<void>());

	std::vector<averisera::ObjectValue> vector(2);
	vector[0] = 0.5;
	vector[1] = "FOO";
	averisera::ObjectValue ov_vec(vector);
	ASSERT_EQ(vector[0].as_double(), ov_vec.as_vector()[0].as_double());
	ASSERT_EQ(vector[1].as_string(), ov_vec.as_vector()[1].as_string());

    vector[1] = -0.5;
    ov_vec = averisera::ObjectValue(vector);
    std::vector<double> vd = ov_vec.as_double_vector();
    ASSERT_EQ(std::vector<double>({ 0.5, -0.5 }), vd);
}

TEST(ObjectValue, DefaultConstructor) {
	averisera::ObjectValue bv;
	ASSERT_EQ(averisera::ObjectValue::Type::NONE, bv.type());
}

TEST(ObjectValue, CopyConstructor) {
	const averisera::ObjectValue ov_double(0.5);
	averisera::ObjectValue ov_double2(ov_double);
	EXPECT_EQ(averisera::ObjectValue::Type::DOUBLE, ov_double2.type());
	EXPECT_EQ(0.5, ov_double2.as_double());
}

TEST(ObjectValue, MoveConstructor) {
	averisera::ObjectValue ov_double(0.5);
	averisera::ObjectValue ov_double2(std::move(ov_double));
	EXPECT_EQ(averisera::ObjectValue::Type::DOUBLE, ov_double2.type());
	EXPECT_EQ(0.5, ov_double2.as_double());
	EXPECT_EQ(averisera::ObjectValue::Type::NONE, ov_double.type());
}

TEST(ObjectValue, Assignment) {
	const averisera::ObjectValue ov_double(0.5);
	averisera::ObjectValue ov_int(-1);
	ov_int = ov_double;
	EXPECT_EQ(averisera::ObjectValue::Type::DOUBLE, ov_int.type());
	EXPECT_EQ(0.5, ov_int.as_double());
}

static std::string ov_type_to_string(averisera::ObjectValue::Type typ) {
	std::stringstream ss;
	ss << typ;
	return ss.str();
}

TEST(ObjectValue, TypeToString) {
	ASSERT_EQ("DOUBLE", ov_type_to_string(averisera::ObjectValue::Type::DOUBLE));
	ASSERT_EQ("INT", ov_type_to_string(averisera::ObjectValue::Type::INT));
	ASSERT_EQ("BOOL", ov_type_to_string(averisera::ObjectValue::Type::BOOL));
	ASSERT_EQ("STRING", ov_type_to_string(averisera::ObjectValue::Type::STRING));
	ASSERT_EQ("OBJECT", ov_type_to_string(averisera::ObjectValue::Type::OBJECT));
	ASSERT_EQ("VECTOR", ov_type_to_string(averisera::ObjectValue::Type::VECTOR));
	ASSERT_EQ("NONE", ov_type_to_string(averisera::ObjectValue::Type::NONE));
}

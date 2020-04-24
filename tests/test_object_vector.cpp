#include <gtest/gtest.h>
#include "core/object_vector.hpp"

using namespace averisera;

TEST(ObjectVector, Default) {
	ObjectVector v;
	ASSERT_TRUE(v.is_null());
	ASSERT_EQ(ObjectVector::Type::NONE, v.type());
	ASSERT_EQ(0, v.size());
	ASSERT_THROW(v.push_back(0.2), std::domain_error);
}

TEST(ObjectVector, TypeFromString) {
    ASSERT_EQ(ObjectVector::Type::NONE, ObjectVector::type_from_string("none"));
    ASSERT_EQ(ObjectVector::Type::DOUBLE, ObjectVector::type_from_string("double"));
	ASSERT_EQ(ObjectVector::Type::FLOAT, ObjectVector::type_from_string("float"));
    ASSERT_EQ(ObjectVector::Type::INT8, ObjectVector::type_from_string("int8"));
	ASSERT_EQ(ObjectVector::Type::INT16, ObjectVector::type_from_string("int16"));
    ASSERT_EQ(ObjectVector::Type::INT32, ObjectVector::type_from_string("int32"));
    ASSERT_EQ(ObjectVector::Type::UINT8, ObjectVector::type_from_string("uint8"));
	ASSERT_EQ(ObjectVector::Type::UINT16, ObjectVector::type_from_string("uint16"));
	ASSERT_EQ(ObjectVector::Type::UINT32, ObjectVector::type_from_string("uint32"));
    ASSERT_THROW(ObjectVector::type_from_string("foo"), std::domain_error);
}

static void test_constructor_with_type(ObjectVector::Type typ) {
	ObjectVector v1(typ);
	ASSERT_EQ(typ, v1.type());
	ASSERT_EQ(0, v1.size());
}

TEST(ObjectVector, ConstructorWithType) {
	test_constructor_with_type(ObjectVector::Type::DOUBLE);
	test_constructor_with_type(ObjectVector::Type::FLOAT);
	test_constructor_with_type(ObjectVector::Type::INT8);
	test_constructor_with_type(ObjectVector::Type::INT16);
	test_constructor_with_type(ObjectVector::Type::INT32);
	test_constructor_with_type(ObjectVector::Type::UINT8);
	test_constructor_with_type(ObjectVector::Type::UINT16);
	test_constructor_with_type(ObjectVector::Type::UINT32);
	test_constructor_with_type(ObjectVector::Type::NONE);
}

template <class V> static void test_constructor_with_values(const V val, ObjectVector::Type expected_type) {
	std::vector<V> vec;
	vec.push_back(val);
	ObjectVector ov(vec);
	ASSERT_FALSE(ov.is_null());
	ASSERT_EQ(1, ov.size());
	ASSERT_EQ(1, ov.as<V>().size());
	ASSERT_EQ(expected_type, ov.type());
	ASSERT_EQ(val, ov.as<V>()[0]);
}

TEST(ObjectVector, ConstructorWithValues) {
	test_constructor_with_values<double>(0.2, ObjectVector::Type::DOUBLE);
	test_constructor_with_values<float>((float)0.1, ObjectVector::Type::FLOAT);
	test_constructor_with_values<int8_t>(2, ObjectVector::Type::INT8);
	test_constructor_with_values<int16_t>(-10, ObjectVector::Type::INT16);
	test_constructor_with_values<int32_t>(-2000, ObjectVector::Type::INT32);
	test_constructor_with_values<uint8_t>(200, ObjectVector::Type::UINT8);
	test_constructor_with_values<uint16_t>(400, ObjectVector::Type::UINT16);
	test_constructor_with_values<uint32_t>(20000, ObjectVector::Type::UINT32);
}

template <class V> static void test_move_constructor_with_values(V val, ObjectVector::Type expected_type) {
	std::vector<V> vec;
	vec.push_back(val);
	ObjectVector ov(std::move(vec));
	ASSERT_TRUE(vec.empty());
	ASSERT_FALSE(ov.is_null());
	ASSERT_EQ(1, ov.size());
	ASSERT_EQ(1, ov.as<V>().size());
	ASSERT_EQ(expected_type, ov.type());
	ASSERT_EQ(val, ov.as<V>()[0]);
}

TEST(ObjectVector, MoveConstructorWithValues) {
	test_move_constructor_with_values<double>(0.2, ObjectVector::Type::DOUBLE);
	test_move_constructor_with_values<float>((float)0.1, ObjectVector::Type::FLOAT);
	test_move_constructor_with_values<int8_t>(2, ObjectVector::Type::INT8);
	test_move_constructor_with_values<int16_t>(10, ObjectVector::Type::INT16);
	test_move_constructor_with_values<int32_t>(-2000, ObjectVector::Type::INT32);
	test_move_constructor_with_values<uint8_t>(200, ObjectVector::Type::UINT8);
	test_move_constructor_with_values<uint16_t>(400, ObjectVector::Type::UINT16);
	test_move_constructor_with_values<uint32_t>(2000, ObjectVector::Type::UINT32);
}

TEST(ObjectVector, Swap) {
	ObjectVector v1;
	ObjectVector v2(std::vector<double>({ 0.2 }));
	ObjectVector v3(std::vector<int32_t>({ 2 }));
	ObjectVector v4(std::vector<int8_t>({ 20 }));
	ObjectVector v5(std::vector<uint8_t>({ 200 }));
	ObjectVector v6(std::vector<uint32_t>({ 2000 }));
	ObjectVector v7(std::vector<float>({ (float)0.1 }));
	ObjectVector v8(std::vector<uint16_t>({ 400 }));
	ASSERT_EQ(1u, v4.size()); // for 32-bit
	ASSERT_EQ(1u, v5.size()); // for 32-bit
	ASSERT_EQ(1u, v6.size()); // for 32-bit
	ASSERT_EQ(1u, v7.size()); // for 32-bit
	ASSERT_EQ(1u, v8.size()); // for 32-bit
	v1.swap(v1);
	v2.swap(v1);
	ASSERT_EQ(ObjectVector::Type::DOUBLE, v1.type());
	ASSERT_EQ(1u, v1.size());
	ASSERT_TRUE(v2.is_null());
	ASSERT_EQ(0.2, v1.as<double>()[0]);
	v3.swap(v4);
	ASSERT_EQ(ObjectVector::Type::INT8, v3.type());
	ASSERT_EQ(1u, v3.size());
	ASSERT_EQ(20, v3.as<int8_t>()[0]);
	ASSERT_EQ(ObjectVector::Type::INT32, v4.type());
	ASSERT_EQ(2, v4.as<int32_t>()[0]);
	v2.swap(v5);
	ASSERT_TRUE(v5.is_null());
	ASSERT_EQ(ObjectVector::Type::UINT8, v2.type());	
	ASSERT_EQ(200u, v2.as<uint8_t>()[0]);
	ASSERT_EQ(1u, v2.size());
	v6.swap(v6);
	ASSERT_EQ(ObjectVector::Type::UINT32, v6.type());
	v4.swap(v6);
	ASSERT_EQ(ObjectVector::Type::UINT32, v4.type());
	ASSERT_EQ(ObjectVector::Type::INT32, v6.type());
	ASSERT_EQ(2, v6.as<int32_t>()[0]);
	ASSERT_EQ(2000, v4.as<uint32_t>()[0]);
	v6.swap(v7);
	ASSERT_EQ(ObjectVector::Type::FLOAT, v6.type());
	ASSERT_EQ(ObjectVector::Type::INT32, v7.type());
	ASSERT_EQ((float)0.1, v6.as<float>()[0]);
	ASSERT_EQ(2, v7.as<int32_t>()[0]);
	v7.swap(v8);
	ASSERT_EQ(ObjectVector::Type::UINT16, v7.type());
	ASSERT_EQ(ObjectVector::Type::INT32, v8.type());
	ASSERT_EQ(2, v8.as<int32_t>()[0]);
	ASSERT_EQ(400, v7.as<uint16_t>()[0]);
}

TEST(ObjectVector, PushBack) {
	ObjectVector v1;
	ObjectVector v2(std::vector<double>({ 0.2 }));
	ObjectVector v3(std::vector<int32_t>({ 2 }));
	ASSERT_THROW(v1.push_back(0.2), std::domain_error);
	ASSERT_THROW(v1.push_back((int64_t)2), std::domain_error);
	v2.push_back(0.3);
	v2.push_back((int64_t)1000);
	ASSERT_EQ(3u, v2.size());
	ASSERT_EQ(0.3, v2.as<double>()[1]);
	ASSERT_EQ(1000, v2.as<double>()[2]);
	v3.push_back(1.3);
	v3.push_back((int64_t)200);
	ASSERT_EQ(3u, v3.size());
	ASSERT_EQ(1, v3.as<int32_t>()[1]);
	ASSERT_EQ(200, v3.as<int32_t>()[2]);
	ObjectVector v4(std::vector<int8_t>({ 20 }));
	ASSERT_THROW(v4.push_back(1000.0), std::out_of_range);
	ASSERT_THROW(v4.push_back((int64_t)1000), std::out_of_range);	
	ObjectVector v5(std::vector<float>({ (float)0.1 }));
	v5.push_back(1.21000000001);
	ASSERT_EQ((float)1.21000000001, v5.as<float>()[1]);
	ObjectVector v6(std::vector<uint16_t>({ 400 }));
	v6.push_back(401);
	ASSERT_EQ(401, v6.as<uint16_t>()[1]);
}

TEST(ObjectVector, MoveConstructor) {
	ObjectVector v1;
	ObjectVector v2(std::vector<double>({ 0.2 }));
	ObjectVector v3(std::vector<int32_t>({ 2 }));
	ObjectVector v4(std::vector<int8_t>({ 20 }));
	ObjectVector v5(std::vector<uint8_t>({ 200 })); // 200u breaks 32-bit build
	ObjectVector v6(std::vector<uint32_t>({ 2000u }));
	ObjectVector v7(std::vector<float>({ (float)0.1 }));
	ObjectVector v8(std::vector<uint16_t>({ 400 }));
	ObjectVector w1(std::move(v1));
	ObjectVector w2(std::move(v2));
	ObjectVector w3(std::move(v3));
	ObjectVector w4(std::move(v4));
	ObjectVector w5(std::move(v5));
	ObjectVector w6(std::move(v6));
	ObjectVector w7(std::move(v7));
	ObjectVector w8(std::move(v8));
	ASSERT_TRUE(v1.is_null());
	ASSERT_TRUE(v2.is_null());
	ASSERT_TRUE(v3.is_null());
	ASSERT_TRUE(v4.is_null());
	ASSERT_TRUE(v5.is_null());
	ASSERT_TRUE(v6.is_null());
	ASSERT_TRUE(v7.is_null());
	ASSERT_TRUE(v8.is_null());
	ASSERT_TRUE(w1.is_null());
	ASSERT_FALSE(w2.is_null());
	ASSERT_FALSE(w3.is_null());
	ASSERT_FALSE(w4.is_null());
	ASSERT_FALSE(w5.is_null());
	ASSERT_FALSE(w6.is_null());
	ASSERT_FALSE(w7.is_null());
	ASSERT_FALSE(w8.is_null());
	ASSERT_EQ(0.2, w2.as<double>()[0]);
	ASSERT_EQ(2, w3.as<int32_t>()[0]);
	ASSERT_EQ(20, w4.as<int8_t>()[0]);
	ASSERT_EQ(200u, w5.as<uint8_t>()[0]);
	ASSERT_EQ(2000u, w6.as<uint32_t>()[0]);
	ASSERT_EQ((float)0.1, w7.as<float>()[0]);
	ASSERT_EQ(400, w8.as<uint16_t>()[0]);
}

TEST(ObjectVector, CopyConstructor) {
	const ObjectVector v1;
	const ObjectVector v2(std::vector<double>({ 0.2 }));
	const ObjectVector v3(std::vector<int32_t>({ 2 }));
	const ObjectVector v4(std::vector<int8_t>({ 20 }));
	const ObjectVector v5(std::vector<uint8_t>({ 200 }));
	const ObjectVector v6(std::vector<uint32_t>({ 2000u }));
	const ObjectVector v7(std::vector<float>({ (float)0.1 }));
	const ObjectVector v8(std::vector<uint16_t>({ 400 }));
	const ObjectVector w1(v1);
	const ObjectVector w2(v2);
	const ObjectVector w3(v3);
	const ObjectVector w4(v4);
	const ObjectVector w5(v5);
	const ObjectVector w6(v6);
	const ObjectVector w7(v7);
	const ObjectVector w8(v8);
	ASSERT_TRUE(w1.is_null());
	ASSERT_FALSE(w2.is_null());
	ASSERT_FALSE(w3.is_null());
	ASSERT_FALSE(w4.is_null());
	ASSERT_FALSE(w5.is_null());
	ASSERT_FALSE(w6.is_null());
	ASSERT_FALSE(w7.is_null());
	ASSERT_FALSE(w8.is_null());
	ASSERT_EQ(0.2, w2.as<double>()[0]);
	ASSERT_EQ(2, w3.as<int32_t>()[0]);
	ASSERT_EQ(20, w4.as<int8_t>()[0]);
	ASSERT_EQ(200u, w5.as<uint8_t>()[0]);
	ASSERT_EQ(2000u, w6.as<uint32_t>()[0]);
	ASSERT_EQ((float)0.1, w7.as<float>()[0]);
	ASSERT_EQ(400, w8.as<uint16_t>()[0]);
}

TEST(ObjectVector, PrintDouble) {
	std::stringstream ss;
	ObjectVector v2(std::vector<double>({ 0.2 }));
	ss << v2;
	ASSERT_EQ("double|[0.2]", ss.str());
}

TEST(ObjectVector, PrintUInt8) {
	std::stringstream ss;
	ObjectVector v2(std::vector<uint8_t>({ 0, 4, 10 }));
	ss << v2;
	ASSERT_EQ("uint8|[0, 4, 10]", ss.str());
}

TEST(ObjectVector, PrintInt8) {
	std::stringstream ss;
	ObjectVector v2(std::vector<int8_t>({ 0, -4, 10 }));
	ss << v2;
	ASSERT_EQ("int8|[0, -4, 10]", ss.str());
}

TEST(ObjectVector, PrintNone) {
	std::stringstream ss;
	ObjectVector v1;
	ss << v1;
	ASSERT_EQ("none", ss.str());	
}

TEST(ObjectVector, GetType) {
	ASSERT_EQ(ObjectVector::Type::DOUBLE, ObjectVector::get_type<double>());
	ASSERT_EQ(ObjectVector::Type::FLOAT, ObjectVector::get_type<float>());
	ASSERT_EQ(ObjectVector::Type::INT16, ObjectVector::get_type<int16_t>());
	ASSERT_EQ(ObjectVector::Type::INT32, ObjectVector::get_type<int32_t>());
	ASSERT_EQ(ObjectVector::Type::INT8, ObjectVector::get_type<int8_t>());
	ASSERT_EQ(ObjectVector::Type::UINT32, ObjectVector::get_type<uint32_t>());
	ASSERT_EQ(ObjectVector::Type::UINT8, ObjectVector::get_type<uint8_t>());
	ASSERT_EQ(ObjectVector::Type::UINT16, ObjectVector::get_type<uint16_t>());
}

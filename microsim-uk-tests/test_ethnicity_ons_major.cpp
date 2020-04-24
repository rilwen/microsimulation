/*
(C) Averisera Ltd 2017
*/
#include <gtest/gtest.h>
#include "microsim-uk/ethnicity/ethnicity_ons_major.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(EthnicityONSMajor, GetSize) {
	ASSERT_EQ(5, Ethnicity::get_size<EthnicityONSMajor>());
}

TEST(EthnicityONSMajor, ClassificationName) {
	ASSERT_STREQ("ONS_MAJOR", Ethnicity::get_classification_name<EthnicityONSMajor>());
}

TEST(EthnicityONSMajor, GetName) {
	ASSERT_STREQ("WHITE", Ethnicity::get_name<EthnicityONSMajor>(EthnicityONSMajor::Group::WHITE));
	ASSERT_STREQ("OTHER", Ethnicity::get_name<EthnicityONSMajor>(EthnicityONSMajor::Group::OTHER));
	ASSERT_STREQ("", Ethnicity::get_name<EthnicityONSMajor>(EthnicityONSMajor::Group::SIZE));
}

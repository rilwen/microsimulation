/*
(C) Averisera Ltd 2017
*/
#include <gtest/gtest.h>
#include "microsim-uk/ethnicity/ethnicity_ons_full.hpp"
#include "microsim-uk/ethnicity/ethnicity_ons_major.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(EthnicityONSFull, ToONSMajor) {
	ASSERT_EQ(EthnicityONSMajor::Group::ASIAN, EthnicityONSFull::to_ons_major(EthnicityONSFull::Group::INDIAN));
	ASSERT_EQ(EthnicityONSMajor::Group::SIZE, EthnicityONSFull::to_ons_major(EthnicityONSFull::Group::SIZE));
}

TEST(EthnicityONSFull, FromONSMajor) {
	typedef EthnicityONSFull::range_type range;
	ASSERT_EQ(range(EthnicityONSFull::Group::INDIAN, EthnicityONSFull::Group::OTHER_ASIAN), EthnicityONSFull::from_ons_major(EthnicityONSMajor::Group::ASIAN));
	ASSERT_EQ(range(EthnicityONSFull::Group::SIZE, EthnicityONSFull::Group::SIZE), EthnicityONSFull::from_ons_major(EthnicityONSMajor::Group::SIZE));
}

TEST(EthnicityONSFull, GetSize) {
	ASSERT_EQ(18, Ethnicity::get_size<EthnicityONSFull>());
}

TEST(EthnicityONSFull, ClassificationName) {
	ASSERT_STREQ("ONS_FULL", Ethnicity::get_classification_name<EthnicityONSFull>());
}

TEST(EthnicityONSFull, GetName) {
	ASSERT_STREQ("WHITE_BRITISH", Ethnicity::get_name<EthnicityONSFull>(EthnicityONSFull::Group::WHITE_BRITISH));
	ASSERT_STREQ("ANY_OTHER", Ethnicity::get_name<EthnicityONSFull>(EthnicityONSFull::Group::ANY_OTHER));
	ASSERT_STREQ("", Ethnicity::get_name<EthnicityONSFull>(EthnicityONSFull::Group::SIZE));
}

TEST(EthnicityONSFull, RangeFromString) {
	typedef Ethnicity::range_type<EthnicityONSFull> range;
	typedef EthnicityONSFull::Group group;
	ASSERT_EQ(range(group::IRISH, group::CHINESE), Ethnicity::range_from_string<EthnicityONSFull>("IRISH - CHINESE "));
	ASSERT_EQ(range(group::INDIAN, group::INDIAN), Ethnicity::range_from_string<EthnicityONSFull>("INDIAN"));
	ASSERT_EQ(range(group::INDIAN, group::ANY_OTHER), Ethnicity::range_from_string<EthnicityONSFull>("INDIAN-"));
	ASSERT_THROW(Ethnicity::range_from_string<EthnicityONSFull>(""), DataException);
	ASSERT_THROW(Ethnicity::range_from_string<EthnicityONSFull>("-"), DataException);
	ASSERT_THROW(Ethnicity::range_from_string<EthnicityONSFull>("-ARAB"), DataException);
	ASSERT_EQ(range(group::WHITE_BRITISH, group::ANY_OTHER), Ethnicity::range_from_string<EthnicityONSFull>("ALL"));
	ASSERT_EQ(range(group::SIZE, group::SIZE), Ethnicity::range_from_string<EthnicityONSFull>("ALL_OTHERS"));
}

TEST(EthnicityONSFull, RangeToSet) {
	typedef Ethnicity::range_type<EthnicityONSFull> range;
	typedef EthnicityONSFull::Group group;
	ASSERT_EQ(Ethnicity::set_type<EthnicityONSFull>({ group::INDIAN, group::PAKISTANI, group::BANGLADESHI }), Ethnicity::range_to_set<EthnicityONSFull>(range(group::INDIAN, group::BANGLADESHI)));
	ASSERT_EQ(Ethnicity::set_type<EthnicityONSFull>({ group::INDIAN }), Ethnicity::range_to_set<EthnicityONSFull>(range(group::INDIAN, group::INDIAN)));
}

TEST(EthnicityONSFull, RangesToSets) {
	typedef Ethnicity::range_type<EthnicityONSFull> range;
	typedef EthnicityONSFull::Group group;
	typedef Ethnicity::set_type<EthnicityONSFull> set;
	std::vector<range> ranges({ range(group::INDIAN, group::CARIBBEAN), range(group::IRISH, group::IRISH), range(group::ARAB, group::SIZE) });
	std::vector<set> expected_sets({
		Ethnicity::range_to_set<EthnicityONSFull>(ranges[0]),
		Ethnicity::range_to_set<EthnicityONSFull>(ranges[1]),
		Ethnicity::range_to_set<EthnicityONSFull>(ranges[2])
	});
	ASSERT_EQ(expected_sets, Ethnicity::ranges_to_sets<EthnicityONSFull>(ranges));
	ranges.push_back(range(group::SIZE, group::SIZE));
	expected_sets.push_back(set({ group::WHITE_BRITISH, group::GYPSY_OR_TRAVELLER, group::OTHER_WHITE, group::WHITE_AND_ASIAN, group::WHITE_AND_BLACK_CARIBBEAN, group::WHITE_AND_BLACK_AFRICAN, group::OTHER_MIXED, group::OTHER_BLACK }));
	ASSERT_EQ(expected_sets, Ethnicity::ranges_to_sets<EthnicityONSFull>(ranges));
	ranges.push_back(range(group::SIZE, group::SIZE));
	ASSERT_THROW(Ethnicity::ranges_to_sets<EthnicityONSFull>(ranges), DataException);
}

#include <gtest/gtest.h>
#include "microsim-core/ethnicity.hpp"
#include "microsim-core/person_attributes.hpp"

using namespace averisera;
using namespace averisera::microsim;

/** Mock ethnic group classification for testing */
struct EthnicityMock {
	enum class Group : PersonAttributes::ethnicity_t {
		PINK = 0,
		SALMON,
		CHAMPAGNE,
		STRIPED,
		SIZE /**< Use it to get the number of other values */
	};

	/** Number of groups */
	static const size_t SIZE = static_cast<size_t>(Group::SIZE);

	/** Name of the classification scheme */
	static const char* const CLASSIFICATION_NAME;

	/** Names of groups */
	static const std::array<const char*, SIZE + 1> NAMES;
};

const char* const EthnicityMock::CLASSIFICATION_NAME = "MOCK";

const std::array<const char*, EthnicityMock::SIZE + 1> EthnicityMock::NAMES = {
	"PINK",
	"SALMON",
	"CHAMPAGNE",
	"STRIPED",
	"" /**< SIZE */
};

std::ostream& operator<<(std::ostream& os, EthnicityMock::Group group) {
	os << Ethnicity::get_name<EthnicityMock>(group);
	return os;
}

TEST(Ethnicity, ClassificationName) {
	ASSERT_STREQ("MOCK", Ethnicity::get_classification_name<EthnicityMock>());
}

TEST(Ethnicity, GetName) {
	ASSERT_STREQ("PINK", Ethnicity::get_name<EthnicityMock>(EthnicityMock::Group::PINK));
	ASSERT_STREQ("STRIPED", Ethnicity::get_name<EthnicityMock>(EthnicityMock::Group::STRIPED));
	ASSERT_STREQ("", Ethnicity::get_name<EthnicityMock>(EthnicityMock::Group::SIZE));
}

TEST(Ethnicity, GetGroup) {
	ASSERT_EQ(EthnicityMock::Group::PINK, Ethnicity::get_group<EthnicityMock>("PINK"));
	ASSERT_EQ(EthnicityMock::Group::SIZE, Ethnicity::get_group<EthnicityMock>("fooo"));
}

TEST(Ethnicity, RangeFromString) {
	typedef Ethnicity::range_type<EthnicityMock> range;
	typedef EthnicityMock::Group group;
	ASSERT_EQ(range(group::PINK, group::CHAMPAGNE), Ethnicity::range_from_string<EthnicityMock>("PINK - CHAMPAGNE"));
	ASSERT_EQ(range(group::STRIPED, group::STRIPED), Ethnicity::range_from_string<EthnicityMock>("STRIPED"));
	ASSERT_EQ(range(group::SALMON, group::STRIPED), Ethnicity::range_from_string<EthnicityMock>("SALMON-"));
	ASSERT_THROW(Ethnicity::range_from_string<EthnicityMock>(""), DataException);
	ASSERT_THROW(Ethnicity::range_from_string<EthnicityMock>("-"), DataException);
	ASSERT_THROW(Ethnicity::range_from_string<EthnicityMock>("-SALMON"), DataException);
	ASSERT_EQ(range(group::PINK, group::STRIPED), Ethnicity::range_from_string<EthnicityMock>("ALL"));
	ASSERT_EQ(range(group::SIZE, group::SIZE), Ethnicity::range_from_string<EthnicityMock>("ALL_OTHERS"));
}

TEST(Ethnicity, RangeToSet) {
	typedef Ethnicity::range_type<EthnicityMock> range;
	typedef EthnicityMock::Group group;
	ASSERT_EQ(Ethnicity::set_type<EthnicityMock>({ group::PINK, group::SALMON, group::CHAMPAGNE}), Ethnicity::range_to_set<EthnicityMock>(range(group::PINK, group::CHAMPAGNE)));
	ASSERT_EQ(Ethnicity::set_type<EthnicityMock>({ group::SALMON }), Ethnicity::range_to_set<EthnicityMock>(range(group::SALMON, group::SALMON)));
}

TEST(Ethnicity, RangesToSets) {
	typedef Ethnicity::range_type<EthnicityMock> range;
	typedef EthnicityMock::Group group;
	typedef Ethnicity::set_type<EthnicityMock> set;
	std::vector<range> ranges({ range(group::PINK, group::SALMON), range(group::STRIPED, group::STRIPED), range(group::STRIPED, group::SIZE) });
	std::vector<set> expected_sets({
		Ethnicity::range_to_set<EthnicityMock>(ranges[0]),
		Ethnicity::range_to_set<EthnicityMock>(ranges[1]),
		Ethnicity::range_to_set<EthnicityMock>(ranges[2])
	});
	ASSERT_EQ(expected_sets, Ethnicity::ranges_to_sets<EthnicityMock>(ranges));
	ranges.push_back(range(group::SIZE, group::SIZE));
	expected_sets.push_back(set({ group::CHAMPAGNE }));
	ASSERT_EQ(expected_sets, Ethnicity::ranges_to_sets<EthnicityMock>(ranges));
	ranges.push_back(range(group::SIZE, group::SIZE));
	ASSERT_THROW(Ethnicity::ranges_to_sets<EthnicityMock>(ranges), DataException);
}

TEST(Ethnicity, IndexRangeToSet) {
	Ethnicity::index_range_type rng(3, 6);
	ASSERT_EQ(Ethnicity::index_set_type({ 3, 4, 5, 6 }), Ethnicity::index_range_to_set(rng));
}

TEST(Ethnicity, IndexRangesToSets) {
	typedef Ethnicity::index_range_type range;
	typedef Ethnicity::index_set_type idx_set;
	const Ethnicity::group_index_type size = 14;
	std::vector<range> ranges({ range(2, 4), range(5, 5), range(12, 14) });
	std::vector<idx_set> expected_sets({
		idx_set({2, 3, 4}),
		idx_set({5}),
		idx_set({12, 13, 14})
	});
	ASSERT_EQ(expected_sets, Ethnicity::index_ranges_to_sets(ranges, size));
	ranges.push_back(range(size, size));
	expected_sets.push_back(idx_set({ 0, 1, 6, 7, 8, 9, 10, 11 }));
	ASSERT_EQ(expected_sets, Ethnicity::index_ranges_to_sets(ranges, size));
	ASSERT_THROW(Ethnicity::index_ranges_to_sets(ranges, 6), DataException);
	ranges.push_back(range(size, size));
	ASSERT_THROW(Ethnicity::index_ranges_to_sets(ranges, size), DataException);
}



TEST(Ethnicity, IndexConversions) {
	Ethnicity::IndexConversions ic(Ethnicity::IndexConversions::build<EthnicityMock>());
	ASSERT_STREQ("MOCK", ic.classification_name());
	ASSERT_EQ(4, ic.size());
	ASSERT_EQ("SALMON", ic.name(1));
	ASSERT_EQ(1, ic.index("SALMON"));
	ASSERT_EQ(4, ic.index("JEDI", false));
	ASSERT_THROW(ic.index("JEDI"), std::domain_error);
	typedef Ethnicity::index_range_type range;
	ASSERT_EQ(range(0, 1), ic.index_range_from_string("PINK-SALMON"));
	typedef Ethnicity::index_set_type idx_set;
	std::vector<range> ranges({ range(0, 1), range(4, 4) });
	std::vector<idx_set> expected_sets({
		idx_set({ 0, 1 }),
		idx_set({ 2, 3 })
	});
	ASSERT_EQ(expected_sets, ic.index_ranges_to_sets(ranges));
	ranges.push_back(range(20, 21));	
	ASSERT_THROW(ic.index_ranges_to_sets(ranges), DataException);
}

TEST(Ethnicity, IndexConversionsDefault) {
	Ethnicity::IndexConversions ic;
	ASSERT_EQ(0, ic.size());
	ASSERT_STREQ("", ic.classification_name());
}

// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/sorting.hpp"
#include <array>
#include <utility>

TEST(Sorting, SortIndexValue)
{
	std::vector<averisera::Sorting::index_value_pair<double> > data;
	data.push_back(averisera::Sorting::index_value_pair<double>(0, 2.1));
	data.push_back(averisera::Sorting::index_value_pair<double>(1, -1.0));
	data.push_back(averisera::Sorting::index_value_pair<double>(2, 4.0));
	data.push_back(averisera::Sorting::index_value_pair<double>(3, 3.0));

	averisera::Sorting::sort_index_value(data);
	ASSERT_EQ(1u, data[0].first);
	ASSERT_EQ(-1.0, data[0].second);
	ASSERT_EQ(0u, data[1].first);
	ASSERT_EQ(2.1, data[1].second);
	ASSERT_EQ(3u, data[2].first);
	ASSERT_EQ(3.0, data[2].second);
	ASSERT_EQ(2u, data[3].first);
	ASSERT_EQ(4.0, data[3].second);
}

TEST(Sorting, SortPointers) {
	const std::vector<std::string> strings({ "FFF", "ZZZ", "AAA" });
	std::vector<const std::string*> pointers(strings.size());
	std::transform(strings.begin(), strings.end(), pointers.begin(), [](const std::string& str) { return &str; });
	averisera::Sorting::sort_pointers(pointers);
	ASSERT_EQ(&strings[2], pointers[0]);
	ASSERT_EQ(&strings[0], pointers[1]);
	ASSERT_EQ(&strings[1], pointers[2]);
}

TEST(Sorting, TopologicalSort) {
    typedef std::array<int, 2> value;
    std::vector<value> values;
    const int ignored = -1;
    values.push_back({ 2, 3 });
    values.push_back({ 1, 2 });    
    values.push_back({ 1, 3 });
    values.push_back({ -1, 1 });
    values.push_back({ -1, -1 });
    values.push_back({ 3, 4 });
    const size_t n = values.size();
    auto cmp = [ignored](const value& a, const value& b) {
        const bool a_requires_b = (a[0] == b[1]) && (a[0] != ignored);
        const bool b_requires_a = (b[0] == a[1]) && (b[0] != ignored);
        if (a_requires_b != b_requires_a) {
            return b_requires_a ? -1 : 1;
        } else {
            if (!a_requires_b) {
                return 0;
            } else {
                throw std::runtime_error("Cycle");
            }
        }
    };
    averisera::Sorting::topological_sort(values, cmp);
    ASSERT_EQ(n, values.size());
    for (size_t i = 0; i < n; ++i) {
	for (size_t j = i + 1; j < n; ++j) {
	    ASSERT_NE(values[i], values[j]);
	    ASSERT_TRUE(cmp(values[i], values[j]) <= 0) << values[i][0] << ", " << values[i][1] << " --> " << values[j][0] << ", " << values[j][1] << "\n";
	}
    }
    values.push_back({4, 1});
    ASSERT_THROW(averisera::Sorting::topological_sort(values, cmp), std::runtime_error);
}

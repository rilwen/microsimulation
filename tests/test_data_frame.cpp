#include <gtest/gtest.h>
#include "core/data_frame.hpp"
#include "core/object_value.hpp"

using namespace averisera;

TEST(DataFrame, SortIndex) {
	DataFrame<char, int> df(std::vector<char>({ 'a', 'b' }), std::vector<int>({ 56, 40, 12 }));
	df.ix(0, 0) = 0.0;
	df.ix(0, 1) = 1.0;
	df.ix(1, 0) = 2.0;
	df.ix(1, 1) = 3.0;
	df.ix(2, 0) = 4.0;
	df.ix(2, 1) = 5.0;
	df.sort_index();
	ASSERT_EQ(4., df.ix(0, 0));
	ASSERT_EQ(5., df.ix(0, 1));
	ASSERT_EQ(2., df.ix(1, 0));
	ASSERT_EQ(3., df.ix(1, 1));
	ASSERT_EQ(0., df.ix(2, 0));
	ASSERT_EQ(1., df.ix(2, 1));
	ASSERT_EQ(12, df.index()[0]);
	ASSERT_EQ(40, df.index()[1]);
	ASSERT_EQ(56, df.index()[2]);
}

TEST(DataFrame, SortValues) {
	DataFrame<char, int> df(std::vector<char>({ 'a', 'b' }), std::vector<int>({ 10, 20, 30 }));
	df.ix(0, 0) = 0.0;
	df.ix(0, 1) = 5.0;
	df.ix(1, 0) = 2.0;
	df.ix(1, 1) = 7.0;
	df.ix(2, 0) = 4.0;
	df.ix(2, 1) = 1.0;
	df.sort_values('b');
	ASSERT_EQ(4., df.ix(0, 0));
	ASSERT_EQ(1., df.ix(0, 1));
	ASSERT_EQ(0., df.ix(1, 0));
	ASSERT_EQ(5., df.ix(1, 1));
	ASSERT_EQ(2., df.ix(2, 0));
	ASSERT_EQ(7., df.ix(2, 1));
	ASSERT_EQ(30, df.index()[0]);
	ASSERT_EQ(10, df.index()[1]);
	ASSERT_EQ(20, df.index()[2]);
	df.sort_values('b', false);
	ASSERT_EQ(4., df.ix(2, 0));
	ASSERT_EQ(1., df.ix(2, 1));
	ASSERT_EQ(0., df.ix(1, 0));
	ASSERT_EQ(5., df.ix(1, 1));
	ASSERT_EQ(2., df.ix(0, 0));
	ASSERT_EQ(7., df.ix(0, 1));
	ASSERT_EQ(30, df.index()[2]);
	ASSERT_EQ(10, df.index()[1]);
	ASSERT_EQ(20, df.index()[0]);
	df.sort_values('b', true, [](double x) { return -x; });
	ASSERT_EQ(4., df.ix(2, 0));
	ASSERT_EQ(1., df.ix(2, 1));
	ASSERT_EQ(0., df.ix(1, 0));
	ASSERT_EQ(5., df.ix(1, 1));
	ASSERT_EQ(2., df.ix(0, 0));
	ASSERT_EQ(7., df.ix(0, 1));
	ASSERT_EQ(30, df.index()[2]);
	ASSERT_EQ(10, df.index()[1]);
	ASSERT_EQ(20, df.index()[0]);
}

TEST(DataFrame, DropRow) {
	DataFrame<char, int> df(std::vector<char>({ 'a', 'b' }), std::vector<int>({ 56, 40, 12 }));
	df.ix(0, 0) = 0.0;
	df.ix(0, 1) = 1.0;
	df.ix(1, 0) = 2.0;
	df.ix(1, 1) = 3.0;
	df.ix(2, 0) = 4.0;
	df.ix(2, 1) = 5.0;
	DataFrame<char, int> ndf = df.drop_row(40);
	ASSERT_EQ(2u, ndf.nbr_cols());
	ASSERT_EQ(2u, ndf.nbr_rows());
	ASSERT_EQ(std::vector<int>({ 56, 12 }), ndf.index());
	ASSERT_EQ(df.columns(), ndf.columns());
	ASSERT_EQ(df.row_values_ix(0), ndf.row_values_ix(0));
	ASSERT_EQ(df.row_values_ix(2), ndf.row_values_ix(1));

	ndf = df.drop_row(56);
	ASSERT_EQ(df.row_values_ix(1), ndf.row_values_ix(0));
	ASSERT_EQ(df.row_values_ix(2), ndf.row_values_ix(1));

	ndf = df.drop_row(12);
	ASSERT_EQ(df.row_values_ix(0), ndf.row_values_ix(0));
	ASSERT_EQ(df.row_values_ix(1), ndf.row_values_ix(1));
}

TEST(DataFrame, DropRowIx) {
	DataFrame<char, int> df(std::vector<char>({ 'a', 'b' }), std::vector<int>({ 56, 40, 12 }));
	df.ix(0, 0) = 0.0;
	df.ix(0, 1) = 1.0;
	df.ix(1, 0) = 2.0;
	df.ix(1, 1) = 3.0;
	df.ix(2, 0) = 4.0;
	df.ix(2, 1) = 5.0;
	DataFrame<char, int> ndf = df.drop_row_ix(1);
	ASSERT_EQ(2u, ndf.nbr_cols());
	ASSERT_EQ(2u, ndf.nbr_rows());
	ASSERT_EQ(std::vector<int>({ 56, 12 }), ndf.index());
	ASSERT_EQ(df.columns(), ndf.columns());
	ASSERT_EQ(df.row_values_ix(0), ndf.row_values_ix(0));
	ASSERT_EQ(df.row_values_ix(2), ndf.row_values_ix(1));

	ndf = df.drop_row_ix(0);
	ASSERT_EQ(df.row_values_ix(1), ndf.row_values_ix(0));
	ASSERT_EQ(df.row_values_ix(2), ndf.row_values_ix(1));

	ndf = df.drop_row_ix(2);
	ASSERT_EQ(df.row_values_ix(0), ndf.row_values_ix(0));
	ASSERT_EQ(df.row_values_ix(1), ndf.row_values_ix(1));
}

TEST(DataFrame, SortColumns) {
	DataFrame<char, int> df(std::vector<char>({ 'b', 'a' }), std::vector<int>({ 56, 40, 12 }));
	df.ix(0, 0) = 0.0;
	df.ix(0, 1) = 1.0;
	df.ix(1, 0) = 2.0;
	df.ix(1, 1) = 3.0;
	df.ix(2, 0) = 4.0;
	df.ix(2, 1) = 5.0;
	df.sort_columns();
	ASSERT_EQ(1., df.ix(0, 0));
	ASSERT_EQ(0., df.ix(0, 1));
	ASSERT_EQ(3., df.ix(1, 0));
	ASSERT_EQ(2., df.ix(1, 1));
	ASSERT_EQ(5., df.ix(2, 0));
	ASSERT_EQ(4., df.ix(2, 1));
}

TEST(DataFrame, FindingIndices) {
	DataFrame<char, int> df(std::vector<char>({ 'a', 'b' }), std::vector<int>({ 56, 40, 12 }));
	ASSERT_EQ(0, df.col_idx('a'));
	ASSERT_EQ(1, df.col_idx('b'));
	ASSERT_THROW(df.col_idx('z'), std::out_of_range);
	static const auto not_found = DataFrame<char, int>::NOT_FOUND;
	ASSERT_EQ(not_found, df.col_idx_unsafe('z'));
	ASSERT_EQ(0, df.row_idx(56));
	ASSERT_EQ(1, df.row_idx(40));
	ASSERT_EQ(2, df.row_idx(12));
	ASSERT_THROW(df.row_idx(1000), std::out_of_range);
	ASSERT_EQ(not_found, df.row_idx_unsafe(1000));
}

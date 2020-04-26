// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/data_frame_builder.hpp"
#include "core/data_frame.hpp"

using namespace averisera;

typedef DataFrame<std::string, int> dataframe;

static dataframe make_dataframe() {
	Eigen::MatrixXd values(3, 2);
	values << 0.1, 0.2,
		0.21, 0.31,
		10.1, 20;
	return dataframe(std::move(values), std::vector<std::string>({ "A", "B" }), std::vector<int>({ 1, 2, 4 }));
}

static const dataframe expected(make_dataframe());

TEST(DataFrameBuilder, RowWise) {
	DataFrameBuilder<std::string, int> builder = DataFrameBuilder<std::string, int>::make_rowwise(std::vector<std::string>({ "A", "B" }));
	ASSERT_TRUE(builder.empty());
	ASSERT_EQ(expected.nbr_cols(), builder.nbr_cols());
	ASSERT_EQ(0, builder.nbr_rows());
	builder.add_row(1, std::vector<double>({ 0.1, 0.2 }));
	ASSERT_THROW(builder.add_row(1, std::vector<double>({ 0.1, 0.2, 0.3 })), std::domain_error);
	ASSERT_EQ(1, builder.nbr_rows());
	ASSERT_THROW(builder.add_col("X", { 0.0 }), std::domain_error);
	ASSERT_FALSE(builder.empty());
	builder.add_row(2, { 0.21, 0.31 });
	builder.add_row(4, { 10.1, 20 });
	ASSERT_EQ(expected, builder.build());
}

TEST(DataFrameBuilder, ColWise) {
	DataFrameBuilder<std::string, int> builder = DataFrameBuilder<std::string, int>::make_colwise(std::vector<int>({ 1, 2, 4 }));
	ASSERT_TRUE(builder.empty()); 
	ASSERT_EQ(0, builder.nbr_cols());
	ASSERT_EQ(expected.nbr_rows(), builder.nbr_rows());
	builder.add_col("A", { 0.1, 0.21, 10.1 });
	ASSERT_THROW(builder.add_col("A", { 0.1, 0.21 }), std::domain_error);
	ASSERT_EQ(1, builder.nbr_cols());
	ASSERT_THROW(builder.add_row(0, { 0.0 }), std::domain_error);
	ASSERT_FALSE(builder.empty());
	builder.add_col("B", { 0.2, 0.31, 20 });
	ASSERT_EQ(expected, builder.build());
}

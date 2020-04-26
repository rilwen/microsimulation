// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/column_filter.hpp"
#include "core/stream_data_output.hpp"
#include "core/csv_file_reader.hpp"
#include "testing/temporary_file.hpp"
#include <sstream>
#include <vector>
#include <stdexcept>

TEST(ColumnFilter, Read) {
	averisera::testing::TemporaryFileWithData tmp("idx\tage[idx]\tweight[idx]\n"
		"1\t12\t200\n"
		"\n"
		"2\t76\t12\n"
		"3\t18\t999\n");
	std::stringstream ss;
	averisera::CSVFileReader reader(tmp.filename);
	averisera::StreamDataOutput output(&ss);
		
	std::vector<std::string> selected_names(2);
	selected_names[0] = "idx";
	selected_names[1] = "weight[idx]";
	averisera::ColumnFilter column_filter(selected_names);
	column_filter.apply(reader, output);
	ASSERT_EQ("idx\tweight[idx]\n1\t200\n2\t12\n3\t999\n", ss.str());
}

TEST(ColumnFilter, NotFound) {
	averisera::testing::TemporaryFileWithData tmp("idx\tage[idx]\tweight[idx]\n"
		"1\t12\t200\n"
		"\n"
		"2\t76\t12\n"
		"3\t18\t999\n");
	std::stringstream ss;
	averisera::CSVFileReader reader(tmp.filename);
	averisera::StreamDataOutput output(&ss);
		
	std::vector<std::string> selected_names(1);
	selected_names[0] = "foo";
	averisera::ColumnFilter column_filter(selected_names);
	ASSERT_THROW(column_filter.apply(reader, output), std::exception);
}



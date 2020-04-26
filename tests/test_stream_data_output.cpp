// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/stream_data_output.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <sstream>

TEST(StreamDataOutput,Test) {
	// prepare test data
	std::vector<std::string> names(2);
	names[0] = "Idx";
	names[1] = "Age[Idx]";
	std::vector<double> row(2);
	row[0] = 0;
	row[1] = 65.00000000002;
	
	std::stringstream ss;
	// print to standard output, separating elements with a colon and use 16 digits precision
	std::shared_ptr<averisera::DataOutput> data_output(new averisera::StreamDataOutput(&ss, ':', 16));
	
	// print column names first
	data_output->output_column_names(names);
	
	// print a data row
	data_output->output_data_row(row);

	std::stringstream ss2;
	ss2 << "Idx:Age[Idx]\n";
	ss2 << 0 << ":" << std::setprecision(16) << row[1] << "\n";

	ASSERT_EQ(ss2.str(), ss.str());
}

#include <gtest/gtest.h>
#include <cstdio>
#include <numeric>
#include <string>
#include "core/csv_file_reader.hpp"
#include "core/stl_utils.hpp"
#include "testing/temporary_file.hpp"

using namespace averisera;

struct TemporaryFileWithData: public averisera::testing::TemporaryFile {
	TemporaryFileWithData(bool with_names) {
		std::ofstream outf(filename);

		if (with_names) {
			outf << "A\tB\tC\n";
		}
		outf << "0.1\t0.2\t0.3\n";
		outf << "1\t2\t3";
	}
};

TEST(CSVFileReader,WithNames) {
	TemporaryFileWithData tmp(true);

	CSVFileReader reader(tmp.filename);

	ASSERT_TRUE(reader.has_names());

	EXPECT_EQ(3u, reader.count_columns());
	EXPECT_EQ(2u, reader.count_data_rows());

	const std::vector<std::string> colnames = reader.read_column_names();
	EXPECT_EQ(3u, colnames.size());
	EXPECT_EQ("A", colnames[0]);
	EXPECT_EQ("B", colnames[1]);
	EXPECT_EQ("C", colnames[2]);
	EXPECT_TRUE(reader.at_data());

	std::vector<double> row;
	EXPECT_TRUE(reader.has_next_data_row());
	reader.read_data_row(row);
	EXPECT_EQ(0.1, row[0]); EXPECT_EQ(0.2, row[1]); EXPECT_EQ(0.3, row[2]);
	EXPECT_EQ(3u, row.size());
	EXPECT_TRUE(reader.has_next_data_row());
	reader.read_data_row(row);
	EXPECT_EQ(3u, row.size());
	EXPECT_EQ(1, row[0]); EXPECT_EQ(2, row[1]); EXPECT_EQ(3, row[2]);
	EXPECT_FALSE(reader.has_next_data_row());	
}

TEST(CSVFileReader,WithoutNames) {
	TemporaryFileWithData tmp(false);

	CSVFileReader reader(tmp.filename, CSV::Delimiter::TAB, CSV::QuoteCharacter::NONE, false);

	ASSERT_FALSE(reader.has_names());

	EXPECT_EQ(3u, reader.count_columns());
	EXPECT_EQ(2u, reader.count_data_rows());

	const std::vector<std::string> colnames = reader.read_column_names();
	EXPECT_EQ(0u, colnames.size());
	EXPECT_TRUE(reader.at_data());
	
	std::vector<double> row;
	EXPECT_TRUE(reader.has_next_data_row());
	reader.read_data_row(row);
	EXPECT_EQ(3u, row.size());
	EXPECT_EQ(0.1, row[0]); EXPECT_EQ(0.2, row[1]); EXPECT_EQ(0.3, row[2]);
	EXPECT_TRUE(reader.has_next_data_row());
	reader.read_data_row(row);
	EXPECT_EQ(3u, row.size());
	EXPECT_EQ(1, row[0]); EXPECT_EQ(2, row[1]); EXPECT_EQ(3, row[2]);
	EXPECT_FALSE(reader.has_next_data_row());	
}

TEST(CSVFileReader, SelectIndices) {
	TemporaryFileWithData tmp(false);
	CSVFileReader reader(tmp.filename, CSV::Delimiter::TAB, CSV::QuoteCharacter::NONE, false);
	reader.select_columns(std::unordered_set<size_t>({ 1 }));
	EXPECT_EQ(1, reader.count_columns());
	const std::vector<std::string> colnames = reader.read_column_names();
	EXPECT_EQ(0u, colnames.size());
	EXPECT_TRUE(reader.at_data());
	std::vector<double> row;
	reader.read_data_row(row);
	EXPECT_EQ(1, row.size());
	EXPECT_EQ(0.2, row[0]);
	ASSERT_THROW(reader.select_columns(std::unordered_set<std::string>({ "FOO" })), std::logic_error);
}

TEST(CSVFileReader, SelectNames) {
	TemporaryFileWithData tmp(true);
	CSVFileReader reader(tmp.filename, CSV::Delimiter::TAB, CSV::QuoteCharacter::NONE, true);
	std::vector<double> row;
	reader.to_data();
	reader.read_data_row(row);
	EXPECT_EQ(3, row.size());
	EXPECT_EQ(0.2, row[1]);
	reader.select_columns(std::unordered_set<std::string>({ "B", "Z" }));	
	EXPECT_TRUE(reader.at_data());	
	reader.read_data_row(row);
	EXPECT_EQ(1, row.size());
	EXPECT_EQ(2, row[0]);	
	EXPECT_EQ(1, reader.count_columns());
	const std::vector<std::string> colnames = reader.read_column_names();
	EXPECT_EQ(1, colnames.size());
}

TEST(CSVFileReader, SelectNames2) {
	TemporaryFileWithData tmp(true);
	CSVFileReader reader(tmp.filename, CSV::Delimiter::TAB, CSV::QuoteCharacter::NONE, true);
	reader.select_columns(std::unordered_set<std::string>({ "B", "Z" }));
	EXPECT_FALSE(reader.at_data());
}

TEST(CSVFileReader, SelectNames3) {
	TemporaryFileWithData tmp(true);
	CSVFileReader reader(tmp.filename, CSV::Delimiter::TAB, CSV::QuoteCharacter::NONE, true);
	reader.select_columns(std::unordered_set<std::string>({ "B", "Z" }), std::unordered_set<size_t>({ 0 }));
	std::vector<double> row;
	reader.to_data();
	reader.read_data_row(row);
	ASSERT_EQ(2, row.size());
	ASSERT_EQ(std::vector<double>({ 0.1, 0.2 }), row);
}


struct TemporaryFileWithQuotedData: public averisera::testing::TemporaryFile {
	TemporaryFileWithQuotedData(bool with_names) {
		std::ofstream outf(filename);

		if (with_names) {
			outf << "A\tB\tC\n";
		}
		outf << "1\t\"2,3\"\t4\n";        
	}
};

TEST(CSVFileReader,WithNamesQuoted) {
	TemporaryFileWithQuotedData tmp(true);

	CSVFileReader reader(tmp.filename, CSV::Delimiter::TAB, CSV::QuoteCharacter::DOUBLE_QUOTE);

	EXPECT_EQ(3u, reader.count_columns());
	EXPECT_EQ(1u, reader.count_data_rows());

	const std::vector<std::string> colnames = reader.read_column_names();
	EXPECT_EQ(3u, colnames.size());
	EXPECT_EQ("A", colnames[0]);
	EXPECT_EQ("B", colnames[1]);
	EXPECT_EQ("C", colnames[2]);
	EXPECT_TRUE(reader.at_data());

	std::vector<std::string> row;
	EXPECT_TRUE(reader.has_next_data_row());
	reader.read_data_row(row);
	EXPECT_EQ("1", row[0]); EXPECT_EQ("2,3", row[1]); EXPECT_EQ("4", row[2]);
	EXPECT_EQ(3u, row.size());
	EXPECT_TRUE(reader.has_next_data_row());
    reader.read_data_row(row);
    ASSERT_TRUE(row.empty());
}

TEST(CSVFileReader, MapNamesToIndices) {
    auto m = CSVFileReader::map_names_to_indices({"A", "C", "B"});
    ASSERT_EQ(3u, m.size());
    ASSERT_EQ(0u, m["A"]);
    ASSERT_EQ(2u, m["B"]);
    ASSERT_EQ(1u, m["C"]);
    ASSERT_THROW(CSVFileReader::map_names_to_indices({"A", "C", "C"}), std::runtime_error);
}

struct TemporaryFileWithMissingData: public averisera::testing::TemporaryFile {
	TemporaryFileWithMissingData(bool with_names) {
		std::ofstream outf(filename);

		if (with_names) {
			outf << "A\tB\tC\n";
		}
		outf << "0.1\t0.2\t0.3\n";
		outf << "1\t2";
	}
};

TEST(CSVFileReader, LoadToMap) {
    TemporaryFileWithMissingData tmp(true);
	CSVFileReader reader(tmp.filename);
    const std::vector<std::string> colnames = reader.read_column_names();
    const CSVFileReader::index_map_type index_map = CSVFileReader::map_names_to_indices(colnames);
    CSVFileReader::value_map_type value_map;
    reader.read_data_row(index_map, value_map);
    ASSERT_EQ(3u, value_map.size());
    ASSERT_EQ("0.1", value_map["A"]);
    ASSERT_EQ("0.2", value_map["B"]);
    ASSERT_EQ("0.3", value_map["C"]);
    value_map["OTHER"] = "-1";
    reader.read_data_row(index_map, value_map);
    ASSERT_EQ(3u, value_map.size());
    ASSERT_EQ("-1", value_map["OTHER"]);
    ASSERT_EQ("1", value_map["A"]);
    ASSERT_EQ("2", value_map["B"]);
}

TEST(CSVFileReader, LineIterator) {
	TemporaryFileWithData tmp(true);
	CSVFileReader reader(tmp.filename);
	const CSVFileReader::LineIterator end = reader.end();
	CSVFileReader::LineIterator begin = reader.begin();
	ASSERT_NE(begin, end);
	std::string suma;
	while (begin != end) {
		suma = std::accumulate((*begin).begin(), begin->end(), suma);
		++begin;
	}
	ASSERT_EQ("0.10.20.3123", suma);
}

TEST(CSVFileReader, StringIterator) {
	TemporaryFileWithData tmp(true);
	CSVFileReader reader(tmp.filename);
	const CSVFileReader::StringIterator end = reader.end(0);
	CSVFileReader::StringIterator begin = reader.begin(0);
	ASSERT_NE(begin, end);
	ASSERT_NE(begin, reader.begin(1));
	std::string suma;
	while (begin != end) {
		suma = std::accumulate((*begin).begin(), begin->end(), suma);
		++begin;
	}
	ASSERT_EQ("0.11", suma);
}

TEST(CSVFileReader, DoubleIterator) {
	TemporaryFileWithData tmp(true);
	CSVFileReader reader(tmp.filename);
	const CSVFileReader::DoubleIterator end = reader.end_double(1);
	CSVFileReader::DoubleIterator begin = reader.begin_double(1);
	ASSERT_NE(begin, end);
	ASSERT_NE(begin, reader.begin_double(0));
	std::vector<double> values;
	while (begin != end) {
		values.push_back(*begin);
		++begin;
	}
	ASSERT_EQ(std::vector<double>({ 0.2, 2 }), values);
	std::vector<double> column(2);
	std::copy(reader.begin_double(1), reader.end_double(1), column.begin());
	ASSERT_EQ(values, column);
	begin = reader.begin_double(1);
	ASSERT_EQ(0.2, *begin);
	ASSERT_EQ(0.2, *begin++);
	ASSERT_EQ(2.0, *begin);
}

TEST(CSVFileReader, ReadDoubleIndexed) {
	TemporaryFileWithMissingData tmp(true);
	CSVFileReader reader(tmp.filename);
	std::vector<CSVFileReader::index_type> indices({ 0, 2 });
	std::vector<double> row;
	reader.to_data();
	ASSERT_TRUE(reader.read_data_row(indices, true, row));
	ASSERT_EQ(std::vector<double>({ 0.1, 0.3 }), row);
	ASSERT_TRUE(reader.read_data_row(indices, true, row));
	ASSERT_EQ(1.0, row[0]);
	ASSERT_TRUE(std::isnan(row[1]));
	ASSERT_FALSE(reader.has_next_data_row());
	reader.to_data();
	ASSERT_TRUE(reader.read_data_row(indices, false, row));
	ASSERT_THROW(reader.read_data_row(indices, false, row), std::runtime_error);
}

TEST(CSVFileReader, MultiDoubleIteratorFill) {
	TemporaryFileWithMissingData tmp(true);
	CSVFileReader reader(tmp.filename);
	const std::vector<CSVFileReader::index_type> indices({ 0, 2 });
	const CSVFileReader::MultiDoubleIterator end = reader.end_double(indices, true);
	CSVFileReader::MultiDoubleIterator begin = reader.begin_double(indices, true);
	ASSERT_NE(begin, end);
	ASSERT_NE(begin, reader.begin_double(indices, false));
	ASSERT_NE(begin, reader.begin_double(std::vector<CSVFileReader::index_type>({ 1 }), true));
	std::vector<double> read_values;
	while (begin != end) {
		read_values.insert(read_values.end(), begin->begin(), begin->end());
		++begin;
	}
	ASSERT_EQ(4u, read_values.size());
	ASSERT_EQ(0.1, read_values[0]);
	ASSERT_EQ(0.3, read_values[1]);
	ASSERT_EQ(1, read_values[2]);
	ASSERT_TRUE(std::isnan(read_values[3]));
}

TEST(CSVFileReader, MultiDoubleIteratorNoFill) {
	TemporaryFileWithMissingData tmp(true);
	CSVFileReader reader(tmp.filename);
	const std::vector<CSVFileReader::index_type> indices({ 0, 2 });	
	const CSVFileReader::MultiDoubleIterator end = reader.end_double(indices, false);
	CSVFileReader::MultiDoubleIterator begin = reader.begin_double(indices, false);	
	ASSERT_NE(begin, end);
	ASSERT_NE(begin, reader.begin_double(indices, true));
	ASSERT_NE(begin, reader.begin_double(std::vector<CSVFileReader::index_type>({ 1 }), false));
	std::vector<double> read_values;
	ASSERT_EQ(std::vector<double>({ 0.1, 0.3 }), *begin);
	ASSERT_THROW(++begin, std::runtime_error);
}

struct TemporaryFileWithDataAndEmptyLines : public averisera::testing::TemporaryFile {
	TemporaryFileWithDataAndEmptyLines(bool with_names) {
		std::ofstream outf(filename);

		if (with_names) {
			outf << "A\tB\tC\n";
		}
		outf << "0.1\t0.2\t0.3\n";
		outf << "\n";
		outf << "1\t2\t3\n\n";
	}
};

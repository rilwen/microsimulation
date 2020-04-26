// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/csv_line_parser.hpp"
#include "core/csv_line_parser_selected_cols.hpp"

using namespace averisera;

TEST(CSVLineParserSelectedCols, Test) {
	CSVLineParserSelectedCols parser(std::unordered_set<AbstractCSVLineParser::col_idx_t>({ 0, 2 }), std::unique_ptr<AbstractCSVLineParser>(new CSVLineParser<',', '"'>()));
	typedef std::vector<std::string> vs;
	ASSERT_EQ(vs({ "a", "c" }), parser.parse("a,b,c,d"));
	vs values;
	parser.parse("a,b,c2,d", values);
	ASSERT_EQ(vs({ "a", "c2" }), values);
	ASSERT_EQ(vs({ "a" }), parser.parse("a,b"));
}

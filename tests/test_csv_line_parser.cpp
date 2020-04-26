// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/csv_line_parser.hpp"

using namespace averisera;

typedef CSVLineParser<',','"'> parser;

TEST(CSVLineParser, CountColumnsZero) {
    ASSERT_EQ(0u, parser::count_columns(""));
}

TEST(CSVLineParser, CountColumnsTwoEmpty) {
    ASSERT_EQ(2u, parser::count_columns(","));
}

TEST(CSVLineParser, CountColumns) {
    ASSERT_EQ(1u, parser::count_columns("\",\""));
    ASSERT_EQ(3u, parser::count_columns("0.12,\"a,b,c\"\",d\",qwe"));
    ASSERT_EQ(3u, parser::count_columns("0.12, \"a,d\",qwe"));
    ASSERT_EQ(3u, parser::count_columns("0.12, \"a,d\" ,qwe"));
    ASSERT_EQ(3u, parser::count_columns("0.12,\"a,b,c\"\",\",qwe"));
}

TEST(CSVLineParser, CountColumnsErrorHandling) {
    ASSERT_THROW(parser::count_columns("\""), std::runtime_error);
    ASSERT_THROW(parser::count_columns("a, \", c"), std::runtime_error);
    ASSERT_THROW(parser::count_columns("\"\\\r"), std::runtime_error);    
}

TEST(CSVLineParser, JustParse) {    
    ASSERT_EQ(std::vector<std::string>(), parser::just_parse(""));
    ASSERT_EQ(std::vector<std::string>({"a"}), parser::just_parse("a"));
    ASSERT_EQ(std::vector<std::string>({"a", "b"}), parser::just_parse("a,b"));
    ASSERT_EQ(std::vector<std::string>({"a", "b,c"}), parser::just_parse("a,\"b,c\""));
    ASSERT_EQ(std::vector<std::string>({"a", "b,c", "\""}), parser::just_parse("a,\"b,c\",\"\"\"\""));
}


TEST(CSVLineParser, Parse) {
    parser p;
    ASSERT_EQ(std::vector<std::string>({"a"}), p.parse("a"));
    ASSERT_EQ(std::vector<std::string>(), p.parse(""));
    ASSERT_EQ(std::vector<std::string>({"a","b","cd"}), p.parse("a,b,cd"));
}

TEST(CSVLineParser, ParseNoQuoting) {
    CSVLineParser<',', 0> p;
    ASSERT_EQ(std::vector<std::string>({"a","b\"","\"cd"}), p.parse("a,b\",\"cd"));
}

TEST(CSVLineParser, TestTabs) {
    CSVLineParser<'\t', 0> p;
    ASSERT_EQ(std::vector<std::string>({"A", "B", "C"}), p.parse("A\tB\tC"));
    ASSERT_EQ(std::vector<std::string>({"A", "B", "C\n"}), p.parse("A\tB\tC\n"));
}

TEST(CSVLineParser, TestTabs2) {
    CSVLineParser<'\t', 0> p;
    std::vector<std::string> elems;
    p.parse("A\tB\tC\n", elems);
    ASSERT_EQ(std::vector<std::string>({"A", "B", "C\n"}), elems);
    p.parse("A\tB\tC", elems);
    ASSERT_EQ(std::vector<std::string>({"A", "B", "C"}), elems);
}

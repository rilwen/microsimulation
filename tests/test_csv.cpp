// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/csv.hpp"
#include "core/csv_line_parser.hpp"

using namespace averisera;

TEST(CSV, MakeAbstract) {
    auto abstract_parser = CSV::make_line_parser(CSV::Delimiter::SEMICOLON, CSV::QuoteCharacter::DOUBLE_QUOTE);
    const auto concrete_ptr1 = dynamic_cast<CSVLineParser<';', '"'>*>(abstract_parser.get());
    ASSERT_NE(nullptr, concrete_ptr1);
    abstract_parser = CSV::make_line_parser(CSV::Delimiter::TAB, CSV::QuoteCharacter::NONE);
    const auto concrete_ptr2 = dynamic_cast<CSVLineParser<'\t', 0>*>(abstract_parser.get());
    ASSERT_NE(nullptr, concrete_ptr2);
}

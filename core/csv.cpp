// (C) Averisera Ltd 2014-2020
#include "csv.hpp"
#include "csv_line_parser.hpp"
#include <iostream>

namespace averisera {
    namespace CSV {

        std::ostream& operator<<(std::ostream& os, Delimiter delim) {
            os << static_cast<char>(delim);
            return os;
        }

        std::ostream& operator<<(std::ostream& os, QuoteCharacter qc) {
            os << static_cast<char>(qc);
            return os;
        }

        
        template <Delimiter delimiter> static std::unique_ptr<AbstractCSVLineParser> make_line_parser(QuoteCharacter quote_character) {
            switch (quote_character) {
            case QuoteCharacter::SINGLE_QUOTE:
                return std::unique_ptr<AbstractCSVLineParser>(new CSVLineParser<static_cast<char>(delimiter), '\''>());
            case QuoteCharacter::DOUBLE_QUOTE:
                return std::unique_ptr<AbstractCSVLineParser>(new CSVLineParser<static_cast<char>(delimiter), '"'>());
            case QuoteCharacter::NONE:
                return std::unique_ptr<AbstractCSVLineParser>(new CSVLineParser<static_cast<char>(delimiter), 0>());
            default:
                throw std::domain_error(boost::str(boost::format("make_csv_line_parser: quote character %c not supported") % quote_character));        
            }
        }
        
        
        std::unique_ptr<AbstractCSVLineParser> make_line_parser(Delimiter delimiter, QuoteCharacter quote_character) {
            switch (delimiter) {
            case Delimiter::COMMA:
                return make_line_parser<Delimiter::COMMA>(quote_character);
            case Delimiter::SEMICOLON:
                return make_line_parser<Delimiter::SEMICOLON>(quote_character);
            case Delimiter::TAB:
                return make_line_parser<Delimiter::TAB>(quote_character);
            default:
                throw std::domain_error(boost::str(boost::format("make_csv_line_parser: delimiter %c not supported") % delimiter));        
            }
        }
    }
}

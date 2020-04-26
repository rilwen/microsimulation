// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_CSV_HPP
#define __AVERISERA_CSV_HPP

#include <iosfwd>
#include <memory>

namespace averisera {
    class AbstractCSVLineParser;
    
    namespace CSV {
        enum class Delimiter : char {
            COMMA = ',',
                SEMICOLON = ';',
                TAB = '\t'
                };

        std::ostream& operator<<(std::ostream& os, Delimiter delim);
        
        enum class QuoteCharacter : char {
            SINGLE_QUOTE = '\'',
                DOUBLE_QUOTE = '"',
                NONE = 0 /**< No quoting in file */
                };

        std::ostream& operator<<(std::ostream& os, QuoteCharacter qc);
        
        /** @throw std::domain_error If delimiter is not one of ",;\t" or quote_character is quote nor double-quote */
        std::unique_ptr<AbstractCSVLineParser> make_line_parser(Delimiter delimiter, QuoteCharacter quote_character);
    }
}

#endif // __AVERISERA_CSV_HPP

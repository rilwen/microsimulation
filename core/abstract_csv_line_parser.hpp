#ifndef __AVERISERA_ABSTRACT_CSV_LINE_PARSER_HPP
#define __AVERISERA_ABSTRACT_CSV_LINE_PARSER_HPP

#include <string>
#include <vector>

namespace averisera {
    /** Abstract CSV file parser */
    class AbstractCSVLineParser {
    public:
		typedef size_t col_idx_t;

        virtual ~AbstractCSVLineParser() {}

        /**
          @param[in] line Input line, assumed to be without end-of-line characters.
          @return Vector of parsed values
         */
        virtual std::vector<std::string> parse(const std::string& line) = 0;

        /** Not threadsafe 
          @param[in] line Input line, assumed to be without end-of-line characters.
          @param[out] values Vector of parsed values
         */
        virtual void parse(const std::string& line, std::vector<std::string>& values) = 0;
    };
}

#endif // __AVERISERA_ABSTRACT_CSV_LINE_PARSER_HPP

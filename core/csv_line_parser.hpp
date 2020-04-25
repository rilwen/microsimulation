#ifndef __AVERISERA_CSV_LINE_PARSER_HPP
#define __AVERISERA_CSV_LINE_PARSER_HPP

#include "abstract_csv_line_parser.hpp"
#include "data_exception.hpp"
#include <cassert>
#include <limits>
#include <boost/format.hpp>

namespace averisera {
    /** Parses a single line from a CSV file. 
      Supports quoted strings with separators and quoted quotes inside. 
      Assumes that newline and carriage return characters have been stripped off the line.
     */
    template <char delimiter, char quote_character> class CSVLineParser: public AbstractCSVLineParser {
    public:
        CSVLineParser() {
        }
        
        CSVLineParser& operator=(const CSVLineParser&) = delete;
        CSVLineParser(const CSVLineParser&) = default;

        /** Count number of columns in line. An empty line is considered to have zero columns. */
        static col_idx_t count_columns(const std::string& line) {
            if (line.empty()) {
                return 0;
            }
            col_idx_t nbr_columns = 1;
            walk_over_line(line.begin(), line.end(), [](char){}, [&nbr_columns](size_t){
                    if (nbr_columns == std::numeric_limits<col_idx_t>::max()) {
                        throw DataException("CSVLineParser: column number overflow");
                    }
                    ++nbr_columns;
                });
            return nbr_columns;
        }

        std::vector<std::string> parse(const std::string& line) override {
            std::vector<std::string> values;
            parse(line, values);
            return values;
        }

        void parse(const std::string& line, std::vector<std::string>& values) override {
            const col_idx_t nbr_cols_in_line = find_column_starts(line);
            values.resize(nbr_cols_in_line);            
            size_t i0 = 0;                
            for (col_idx_t k = 0; k < nbr_cols_in_line; ++k) {
                const size_t i1 = ((k + 1) < nbr_cols_in_line) ? _column_starts[k + 1] : line.size();
                assert(i1 >= i0);
                values[k].clear();
                values[k].reserve(i1 - i0);
                i0 = i1;
            }
			col_idx_t k = 0;
            walk_over_line(line.begin(), line.end(), [&values,&k](char c){ values[k].push_back(c); }, [&k](size_t){++k;});
        }

        static std::vector<std::string> just_parse(const std::string& line) {
            return CSVLineParser<delimiter, quote_character>().parse(line);
        }
    private:
        std::vector<size_t> _column_starts; /**< Starting indices of each column */

        enum class CSVState {
            UnquotedField,
                QuotedField,
                QuotedQuote
                };

        /** @return Number of columns in line */
        col_idx_t find_column_starts(const std::string& line) {
            if (line.empty()) {
                return 0;
            }
            _column_starts.resize(1);
            _column_starts.front() = 0;
            col_idx_t nbr_columns = 1;
            walk_over_line(line.begin(), line.end(), [](char){}, [&](size_t col_start){
                    if (nbr_columns == std::numeric_limits<col_idx_t>::max()) {
                        throw DataException("CSVLineParser: column number overflow");
                    }
                    ++nbr_columns;
                    _column_starts.push_back(col_start);
                });
            return nbr_columns;
        }

        // on_char(char c)
        // on_delim(size_t delim_position)
        template <class OnChar, class OnDelim> static void walk_over_line(const std::string::const_iterator i0, const std::string::const_iterator i1, OnChar on_char, OnDelim on_delim) {
            // based on http://stackoverflow.com/a/30338543/59557
            CSVState state = CSVState::UnquotedField;
            for (auto i = i0; i != i1; ++i) {
                const char c = *i;
                switch (state) {
                case CSVState::UnquotedField:
                    switch (c) {
                    case delimiter: // end of field
                        on_delim(static_cast<size_t>(std::distance(i0, i)));
                        break;
                    case quote_character:
                        state = CSVState::QuotedField;
                        break;
                    default:
                        on_char(c);
                        break;
                    }
                    break;
                case CSVState::QuotedField:
                    switch(c) {
                    case quote_character:
                        state = CSVState::QuotedQuote;
                        break;
                    default:
                        on_char(c);
                        break;
                    }
                    break;
                default:
                    assert(CSVState::QuotedQuote == state);
                    switch (c) {
                    case delimiter: // delimiter after closing quote
                        on_delim(static_cast<size_t>(std::distance(i0, i)));
                        state = CSVState::UnquotedField;
                        break;
                    case quote_character: // e.g. "" -> "
                        on_char(quote_character);
                        state = CSVState::QuotedField;
                        break;
                    default: // end of quote
                        state = CSVState::UnquotedField;
                        break;
                    }
                }
            }
            if (state == CSVState::QuotedField) {
                const std::string line(i0, i1);
                throw DataException(boost::str(boost::format("CSVLineParser: unterminated quote %c in line %s") % quote_character % line.substr(0, 80)));
            }
        }
    };

}

#endif // __AVERISERA_CSV_LINE_PARSER_HPP

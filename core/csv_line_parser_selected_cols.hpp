// (C) Averisera Ltd 2014-2020
#pragma once
#include "abstract_csv_line_parser.hpp"
#include <memory>
#include <unordered_set>
#include <vector>

namespace averisera {
	/** Parser which reads only columns with selected indices using another parser */
	class CSVLineParserSelectedCols: public AbstractCSVLineParser {
	public:
		/** @throw std::domain_error if parser is null */
		CSVLineParserSelectedCols(const std::unordered_set<col_idx_t>& sel_cols, std::unique_ptr<AbstractCSVLineParser>&& parser);

		std::vector<std::string> parse(const std::string& line) override;

		void parse(const std::string& line, std::vector<std::string>& values) override;

	private:
		std::vector<col_idx_t> sel_cols_;
		std::unique_ptr<AbstractCSVLineParser> parser_;
	};
}

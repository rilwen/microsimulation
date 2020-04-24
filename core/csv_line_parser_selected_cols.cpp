#include "csv_line_parser_selected_cols.hpp"
#include "preconditions.hpp"
#include <algorithm>

namespace averisera {
	CSVLineParserSelectedCols::CSVLineParserSelectedCols(const std::unordered_set<col_idx_t>& sel_cols, std::unique_ptr<AbstractCSVLineParser>&& parser)
		: sel_cols_(sel_cols.begin(), sel_cols.end()), parser_(std::move(parser)) {
		check_not_null(parser_, "CSVLineParserSelectedCols: null parser");
		std::sort(sel_cols_.begin(), sel_cols_.end());
	}

	std::vector<std::string> CSVLineParserSelectedCols::parse(const std::string& line) {
		std::vector<std::string> parsed;
		parse(line, parsed);
		return parsed;
	}

	void CSVLineParserSelectedCols::parse(const std::string& line, std::vector<std::string>& values) {
		std::vector<std::string> full;
		parser_->parse(line, full);
		auto it = std::lower_bound(sel_cols_.begin(), sel_cols_.end(), full.size());
		const size_t n = std::distance(sel_cols_.begin(), it); // how many selected columns we got this time
		values.resize(n);
		it = sel_cols_.begin();
		for (auto dst_it = values.begin(); dst_it != values.end(); ++dst_it, ++it) {
			assert(it != sel_cols_.end());
			*dst_it = full[*it];
		}
	}
}

/*
(C) Averisera Ltd 2014-2020
*/
#ifndef __AVERISERA_COLUMN_FILTER_H
#define __AVERISERA_COLUMN_FILTER_H

#include "csv_file_filter.hpp"
#include <string>
#include <vector>

namespace averisera {
	/** Prints selected columns. Optionally can reject rows which have negative values (== special codes) in them. */
	class ColumnFilter: public CSVFileFilter {
		public:
			struct Range {
				/** constructs a Range with given parameters */
				Range(const std::string& name, double low, double up);
				/** default constructor - empty range */
				Range();

				std::string column_name; /** column the range applies to */
				double lower; /** inclusive */
				double upper; /** inclusive */
			};
			/** selected_columns - names of selected columns */
			/** reject negative - reject rows with negative values (error codes) */
			ColumnFilter(const std::vector<std::string>& selected_columns, bool reject_negative = true,
				const std::vector<Range>& selected_ranges = std::vector<Range>());

			/** Select all columns */
			ColumnFilter(bool reject_negative = true);

			ColumnFilter& operator=(const ColumnFilter&) = delete;
			
			void apply(CSVFileReader& reader, DataOutput& output);
		private:
			std::vector<std::string> _selected_columns; /** selected column names */
			std::vector<Range> _selected_ranges; /** vector of ranges allowed in some columns */
			const bool _reject_negative;
	};
}

#endif

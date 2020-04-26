/*
(C) Averisera Ltd 2014-2020
*/
#include "column_filter.hpp"
#include "csv_file_reader.hpp"
#include "data_output.hpp"
#include "log.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <boost/lexical_cast.hpp>

namespace averisera {
	ColumnFilter::ColumnFilter(const std::vector<std::string>& selected_columns, bool reject_negative, const std::vector<Range>& selected_ranges)
		: _selected_columns(selected_columns), _selected_ranges(selected_ranges), _reject_negative(reject_negative)
	{
	}

	ColumnFilter::ColumnFilter(bool reject_negative)
		: _reject_negative(reject_negative)
	{
	}
			
	void ColumnFilter::apply(CSVFileReader& reader, DataOutput& output)
	{
		// read provided column names
		const std::vector<std::string> provided_names = reader.read_column_names();
		
		std::vector<std::string> found_column_names;
		std::vector<size_t> found_data_indices;
		std::vector<Range> active_ranges; // ranges which can be applied to provided data, equal to _selected_ranges with all ranges referring to non-existent columns omitted
		std::vector<size_t> active_range_column_indices; // indices of columns which active ranges refer to
		//const size_t nbr_provided = provided_names.size();						
		if (!_selected_columns.empty()) {
			// among the provided columns (taken from the reader) find those which are selected
			const size_t nbr_selected = _selected_columns.size();			
			for (size_t i = 0; i < nbr_selected; ++i) {
				//bool found = false;
				/*
				for (size_t j = 0; j < nbr_provided; ++j) {
					if (_selected_columns[i] == provided_names[j]) {
						found = true;
						index = j;
						break; // no need to look further
					}
				}
				*/
				const std::vector<std::string>::const_iterator iter = std::find(provided_names.begin(), provided_names.end(), _selected_columns[i]);
				if (iter != provided_names.end()) { // we found the selected name; iter points to it
					const size_t index = iter - provided_names.begin(); // calculate the index as the distance 
					found_column_names.push_back(*iter); // append the found name to the end of the found_column_names vector
					found_data_indices.push_back(index); // append the index of the found column name to the end of the found_data_indices vector
				} else {
					/*for (std::vector<std::string>::const_iterator it = provided_names.begin(); it != provided_names.end(); ++it) {
						std::cerr << *it << "\n";
					}*/
					throw std::runtime_error(std::string("Column not found: ") + _selected_columns[i]);
				}
			}
			if (found_column_names.size() < nbr_selected) {
				throw std::runtime_error("Not all columns found"); // TODO: format the error message
			}
		} else {
			// select all columns
			found_column_names = provided_names;
			const size_t n = found_column_names.size();
			found_data_indices.resize(n);
			for (size_t i = 0; i < n; ++i) {
				found_data_indices[i] = i;			
			}
		}

		if (!_selected_ranges.empty()) {
			const size_t nbr_ranges = _selected_ranges.size();
			for (size_t i = 0; i < nbr_ranges; ++i) {
				const std::vector<std::string>::const_iterator iter = std::find(provided_names.begin(), provided_names.end(), _selected_ranges[i].column_name);
				if (iter != provided_names.end()) { // we found the selected name; iter points to it
					const size_t index = iter - provided_names.begin(); // calculate the index as the distance 

					// range is active
					active_ranges.push_back(_selected_ranges[i]);
					active_range_column_indices.push_back(index);
				}
			}

		}
		
		const size_t nbr_found_columns = found_column_names.size();
		const size_t nbr_active_ranges = active_ranges.size();
		
		// print found column names
		output.output_column_names(found_column_names);
		
		// filter the data
		std::vector<std::string> data_row;		
		std::vector<std::string> filtered_row_str(nbr_found_columns);
		std::vector<double> filtered_row_dbl(nbr_found_columns);
		while (reader.has_next_data_row()) { // read data rows until we reach end of file
			const size_t nbr_read = reader.read_data_row(data_row); // read next data row (saved in data_row) and count how many elements were read in this row (saved in nbr_read)
			if (nbr_read > 0) { // skip empty rows
				// iterate over active ranges
				bool within_ranges = true;
				for (size_t i = 0; i < nbr_active_ranges; ++i) {
					const Range& range = active_ranges[i];
					const size_t ri = active_range_column_indices[i]; // find the position of the column the range applies to
					if (ri < nbr_read) {
						try {
							// convert string to double using Boost library
							const auto value = boost::lexical_cast<double>(data_row[ri]);						
							//check if the number is within the allowed range
							within_ranges &= value >= range.lower && value <= range.upper;
						} catch (std::exception&) {
							LOG_WARN() << "Error reading column " << range.column_name << ": cannot convert " << data_row[ri] << " to value";
						}
					} else {
						throw std::runtime_error("Missing data");
					}
					if (!within_ranges) {
						break; // don't check further when one of the ranges is exceeded
					}
				}
				if (!within_ranges) {
					continue; // skip this row
				}

				// iterate over the columns we want to extract from the file
				for (size_t i = 0; i < nbr_found_columns; ++i) {
					const size_t fi = found_data_indices[i]; // find the position of the i-th extracted column
					if (fi < nbr_read) {
						// the row read contains the requested column
						filtered_row_str[i] = data_row[fi];
					} else {
						//filtered_row[i] = NAN; // replace missing value with NaN
						throw std::runtime_error("Missing data");
					}
				}
				bool found_negative = false; // is any number in the row negative?
				for (size_t i = 0; i < nbr_found_columns; ++i) {
					bool read_ok = true;
					try {
						// convert string to double using Boost library
						filtered_row_dbl[i] = boost::lexical_cast<double>(filtered_row_str[i]);						
					} catch (std::exception&) {
						LOG_WARN() << "Error reading column " << found_column_names[i] << ": cannot convert " << filtered_row_str[i] << " to value";
						filtered_row_dbl[i] = std::numeric_limits<double>::quiet_NaN();
						read_ok = false;
					}
					if (read_ok) { // we read a number
						found_negative |= filtered_row_dbl[i] < 0;
						// here another tests and conditions can be applied to read values
						//if (found_column_names[i] == "age" && (filtered_row_dbl[i] < 21 || filtered_row_dbl[i] > 30)) age_rejected = true;
					//	if (found_column_names[i] == "sex" && filtered_row_dbl[i] == 2) sex_rejected = true;
						
					}
				}
				if (!((found_negative && _reject_negative))) { // TODO: simplify it
					// row is accepted
					output.output_data_row(filtered_row_dbl);
				}
			}			
		}		
	}

	ColumnFilter::Range::Range(const std::string& name, double low, double up)
		: column_name(name), lower(low), upper(up)
	{
	}

	ColumnFilter::Range::Range()
		: lower(-std::numeric_limits<double>::infinity()), upper(std::numeric_limits<double>::infinity())
	{
	}
}

// Author: Agnieszka Werpachowska, 2014

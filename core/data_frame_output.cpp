#include "data_frame_output.hpp"

namespace averisera {
	DataFrameOutput::DataFrameOutput(size_t expected_number_rows)
		: builder_(DataFrameBuilder<std::string, size_t>::make_rowwise(std::vector<std::string>(), expected_number_rows)) {}

	void DataFrameOutput::output_column_names(const std::vector<std::string>& names) {
		builder_.set_columns(names);
	}

	void DataFrameOutput::output_data_row(const std::vector<double>& row) {
		builder_.add_row(builder_.nbr_rows(), row);
	}

	DataFrame<std::string, size_t> DataFrameOutput::get() const {
		return builder_.build();
	}
}

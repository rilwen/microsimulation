// (C) Averisera Ltd 2014-2020
#pragma once
#include "data_output.hpp"
#include "data_frame_builder.hpp"

namespace averisera {
	/*" Outputs data to a DataFrame */
	class DataFrameOutput : public DataOutput {
	public:
		DataFrameOutput(size_t expected_number_rows = 0);

		void output_column_names(const std::vector<std::string>& names) override;

		void output_data_row(const std::vector<double>& row) override;

		/** Get the DataFrame */
		DataFrame<std::string, size_t> get() const;
	private:
		DataFrameBuilder<std::string, size_t> builder_;
	};
}

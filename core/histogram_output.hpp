// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_HISTOGRAM_OUTPUT_H
#define __AVERISERA_HISTOGRAM_OUTPUT_H

#include "data_output.hpp"
#include "histogram.hpp"
#include <vector>
#include <string>

namespace averisera {
	// Enables outputting read data to histograms
	class HistogramOutput: public DataOutput {
		public:
			HistogramOutput(const std::vector<double>& lower, const std::vector<double>& upper, size_t n_bins);
			void output_column_names(const std::vector<std::string>& names);
			void output_data_row(const std::vector<double>& row);
			
			// Return a vector of collected histograms
			const std::vector<Histogram>& histograms() const { return _histograms; }
			
			// Return a vector of column names
			const std::vector<std::string>& names() const { return _names; }
		private:
			std::vector<std::string> _names;
			std::vector<Histogram> _histograms;
	};
}

#endif
// (C) Averisera Ltd 2014-2020
#include "histogram_output.hpp"
#include <stdexcept>
#include <boost/lexical_cast.hpp>

namespace averisera {
	HistogramOutput::HistogramOutput(const std::vector<double>& lower, const std::vector<double>& upper, size_t n_bins)
		: _histograms(lower.size())
	{
		const size_t n = lower.size();
		if (upper.size() != n) {
			throw std::domain_error("Upper and lower bound vectors must have the same size");
		}
		for (size_t i = 0; i < n; ++i) {
			_histograms[i] = Histogram(lower[i], upper[i], n_bins);
		}
	}
	
	void HistogramOutput::output_column_names(const std::vector<std::string>& names)
	{
		_names = names;
	}
	
	void HistogramOutput::output_data_row(const std::vector<double>& row)
	{
		const size_t n = _histograms.size();
		if (row.size() != n) {
			throw std::domain_error(std::string("Wrong row size: ") + boost::lexical_cast<std::string>(row.size()));
		}
		for (size_t i = 0; i < n; ++i) {
			_histograms[i].add(row[i]);
		}
	}
}

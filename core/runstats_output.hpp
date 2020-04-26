// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_RUNSTATS_OUTPUT_H
#define __AVERISERA_RUNSTATS_OUTPUT_H

#include "data_output.hpp"
#include "running_statistics.hpp"
#include <vector>

namespace averisera {
	// Creates running statistics of output data
	template <class T> class RunStatsOutput: public DataOutput {
	public:
		RunStatsOutput(size_t size);
		RunStatsOutput<T>& operator=(const RunStatsOutput&) = delete;

		void output_column_names(const std::vector<std::string>&) {}
		void output_data_row(const std::vector<double>& row);
		const std::vector<RunningStatistics<T>>& stats() const { return _stats; }
	private:
		const size_t _size;
		std::vector<RunningStatistics<T>> _stats;
	};
}

#endif 

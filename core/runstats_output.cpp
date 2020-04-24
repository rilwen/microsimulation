#include "runstats_output.hpp"
#include <stdexcept>
#include <string>
#include <boost/lexical_cast.hpp>

namespace averisera {
	template <class T> RunStatsOutput<T>::RunStatsOutput(size_t size)
		: _size(size), _stats(size)
	{
	}

	template <class T> void RunStatsOutput<T>::output_data_row(const std::vector<double>& row)
	{
		if (row.size() != _size) {
			throw std::domain_error(std::string("Bad row size: ") + boost::lexical_cast<std::string>(row.size()));
		}
		for (size_t i = 0; i < _size; ++i) {
			_stats[i].add(row[i]);
		}
	}

    template class RunStatsOutput<double>;
    template class RunStatsOutput<long double>;
}

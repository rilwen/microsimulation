// (C) Averisera Ltd 2014-2020
#include "joint_cdf_output.hpp"
#include "empirical_cdf.hpp"

namespace averisera {
	JointCDFOutput::JointCDFOutput(const std::vector<std::shared_ptr<EmpiricalCDF> >& cdfs, std::shared_ptr<DataOutput> next)
		: _cdfs(cdfs), _next(next), _dim(cdfs.size()), _probs(cdfs.size())
	{
	}

	void JointCDFOutput::output_column_names(const std::vector<std::string>& names)
	{
		_next->output_column_names(names);
	}

	void JointCDFOutput::output_data_row(const std::vector<double>& row)
	{
		for (size_t i = 0; i < _dim; ++i) {
			_probs[i] = _cdfs[i]->prob(row[i]);
		}
		_next->output_data_row(_probs);
	}
}

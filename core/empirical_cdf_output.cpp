#include "empirical_cdf_output.hpp"
#include <boost/lexical_cast.hpp>
#include <stdexcept>

namespace averisera {

	EmpiricalCDFOutput::EmpiricalCDFOutput(size_t n_cdfs)
		: _calcs(n_cdfs)
	{
	}

	void EmpiricalCDFOutput::output_column_names(const std::vector<std::string>& names)
	{
		_colnames = names;
	}

	void EmpiricalCDFOutput::output_data_row(const std::vector<double>& row)
	{
		const size_t n = _calcs.size();
		if (row.size() != n) {
			throw std::domain_error(std::string("Wrong row size: ") + boost::lexical_cast<std::string>(row.size()));
		}
		for (size_t i = 0; i < n; ++i) {
			_calcs[i].add(row[i]);
		}
	}

	std::shared_ptr<EmpiricalCDF> EmpiricalCDFOutput::get_cdf(size_t i)
	{
		if (i >= _calcs.size()) {
			throw std::domain_error("Variable index too large");
		}
		return _calcs[i].calc_cdf();
	}

	std::vector<std::shared_ptr<EmpiricalCDF> > EmpiricalCDFOutput::get_cdfs()
	{
		const size_t n = _calcs.size();
		std::vector<std::shared_ptr<EmpiricalCDF> > cdfs(n);
		for (size_t i = 0; i < n; ++i) {
			cdfs[i] = get_cdf(i);
		}
		return cdfs;
	}
}
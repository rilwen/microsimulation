#ifndef __AVERISERA_EMPIRICAL_CDF_OUTPUT_H
#define __AVERISERA_EMPIRICAL_CDF_OUTPUT_H

#include "data_output.hpp"
#include "empirical_cdf_calc.hpp"
#include <vector>
#include <string>
#include <memory>

namespace averisera {
	class EmpiricalCDF;

	class EmpiricalCDFOutput: public DataOutput {
	public:
		// Prepare output for n_cdfs variables
		EmpiricalCDFOutput(size_t n_cdfs);
		void output_column_names(const std::vector<std::string>& names);
		void output_data_row(const std::vector<double>& row);
		// Return a pointer to completed empirical CDF. We use pointers to avoid copying long data arrays.
		std::shared_ptr<EmpiricalCDF> get_cdf(size_t i);
		// Return pointers to completed empirical CDFs.
		std::vector<std::shared_ptr<EmpiricalCDF> > get_cdfs();
	private:
		std::vector<std::string> _colnames;
		std::vector<EmpiricalCDFCalculator> _calcs;
	};
}

#endif

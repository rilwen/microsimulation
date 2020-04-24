#ifndef __AVERISERA_JOINT_CDF_OUTPUT_H
#define __AVERISERA_JOINT_CDF_OUTPUT_H

#include "data_output.hpp"
#include <vector>
#include <memory>

namespace averisera {
	class EmpiricalCDF;

	// Read samples and convert them to the corresponding values of their empirical CDFs: (x_1, x_2, ...) --> (P(X_1 <= x_1), P(X_2 <= x_2), ...)
	// And pass them to the next data output
	class JointCDFOutput: public DataOutput {
	public:
		// cdfs - vector of pointers to empirical CDFs
		// next - pointer to next output
		JointCDFOutput(const std::vector<std::shared_ptr<EmpiricalCDF> >& cdfs, std::shared_ptr<DataOutput> next);
		JointCDFOutput& operator=(const JointCDFOutput&) = delete;
		void output_column_names(const std::vector<std::string>& names);
		void output_data_row(const std::vector<double>& row);
	private:		
		std::vector<std::shared_ptr<EmpiricalCDF> > _cdfs;
		std::shared_ptr<DataOutput> _next;		
		const size_t _dim;
		std::vector<double> _probs;
	};
}

#endif 

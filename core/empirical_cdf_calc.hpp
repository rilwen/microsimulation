#ifndef __AVERISERA_EMPIRICAL_CDF_CALCULATOR_H
#define __AVERISERA_EMPIRICAL_CDF_CALCULATOR_H

#include <vector>
#include <utility>
#include <memory>

namespace averisera {
	class EmpiricalCDF;

	// Calculates empirical CDF based on sampled data.
	class EmpiricalCDFCalculator {
	public:
		// Create the calculator.
		// capacity -- number of elements we expected to handle; default value of 0 means we don't know.
		EmpiricalCDFCalculator(size_t capacity = 0u);

		// Add next sample
		void add(double x);

		// Return reference to a sorted vector of accumulated samples
		const std::vector<double>& samples();

		// Write pairs (x_i, P(X <= x_i)) ordered by x_i to vector cdf, resizing it if necessary.
		void calc_cdf(std::vector<std::pair<double, double> >& cdf);

		// Return an empirical CDF object. We use shared pointers to avoid copying large data arrays.
		std::shared_ptr<EmpiricalCDF> calc_cdf();
	private:
		void sort_samples();
		std::vector<double> _samples;
		bool _is_sorted;
	};
}

#endif 

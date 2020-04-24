#include "empirical_cdf_calc.hpp"
#include "empirical_cdf.hpp"
#include <algorithm>

namespace averisera {
	// Create the calculator.
	// capacity -- number of elements we expected to handle; default value of 0 means we don't know.
	EmpiricalCDFCalculator::EmpiricalCDFCalculator(size_t capacity)
		: _is_sorted(true)
	{
		// preallocate memory
		_samples.reserve(capacity);
	}

	// Add next sample
	void EmpiricalCDFCalculator::add(double x)
	{
		// add another sample to the vector
		_samples.push_back(x);
		_is_sorted = false;
	}

	const std::vector<double>& EmpiricalCDFCalculator::samples()
	{
		sort_samples();
		return _samples;
	}

	size_t count_unique_values(const std::vector<double>& samples) {
		size_t cntuniq;
		if (!samples.empty()) {
			// use iterators to speed up the scanning
			std::vector<double>::const_iterator it = samples.begin();
			double prev = *it;
			cntuniq = 1;
			++it;
			while (it != samples.end()) {
				const double current = *it;
				if (current != prev) {
					// new unique value
					++cntuniq;
					prev = current;
				}
				++it;
			}
		} else {
			cntuniq = 0;
		}
		return cntuniq;
	}

	// Write pairs (x_i, P(X <= x_i)) ordered by x_i to vector cdf, resizing it if necessary.
	void EmpiricalCDFCalculator::calc_cdf(std::vector<std::pair<double, double> >& cdf)
	{
		sort_samples();

		const size_t cntuniq = count_unique_values(_samples);

		cdf.clear();
		// preallocate memory
		cdf.reserve(cntuniq);

		// for every unique value x, calculate P(X <= x)
		if (cntuniq > 0) {
			const size_t n_tot = _samples.size();
			// use iterators to speed up the scanning
			std::vector<double>::const_iterator it = _samples.begin();
			double prev = *it;
			++it;
			size_t n_occ = 1; // number of occurences of current value AND lower values
			while (it != _samples.end()) {
				const double current = *it;
				if (current != prev) {// new unique value
					// save CDF for old value
					cdf.push_back(std::pair<double, double>(prev, static_cast<double>(n_occ) / static_cast<double>(n_tot)));
					// start tracking the new value
					prev = current;
				}
				++it;
				++n_occ;
			}
			// add the last value
			cdf.push_back(std::pair<double,double>(prev, 1.0));
		}
	}

	std::shared_ptr<EmpiricalCDF> EmpiricalCDFCalculator::calc_cdf()
	{
		std::vector<std::pair<double, double> > cdf;
		calc_cdf(cdf);
		return std::shared_ptr<EmpiricalCDF>(new EmpiricalCDF(cdf));
	}

	void EmpiricalCDFCalculator::sort_samples()
	{
		std::sort(_samples.begin(), _samples.end());
		_is_sorted = true;
	}
}

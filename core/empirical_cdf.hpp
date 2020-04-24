#ifndef __AVERISERA_EMPIRICAL_CDF_H
#define __AVERISERA_EMPIRICAL_CDF_H

#include <vector>
#include <utility>

namespace averisera {
	class EmpiricalCDFCalc;

	// Stores and samples the empirical CDF of random variable X
	class EmpiricalCDF {
	public:
		// Create a CDF object from a vector of pairs (x_k, p_k == P(X < x_k)) ordered by x_k
		// discrete - treat the data as a discrete distribution
		EmpiricalCDF(const std::vector<std::pair<double, double> >& cdf, bool discrete = true);

		// Calculate P(X <= x).
		// If x == x_k, return p_k == P(X < x_k) from the stored vector.
		// If x_{k-1} < x < x_k:
		//		If discrete = TRUE: interpolate linearly between p_{k-1} and p_k
		//		If discrete = FALSE: return p_{k-1}
		// If x < x_0, return 0
		// If x > x_last, return 1.0
		double prob(double x) const;

		// Is the CDF discrete or continuous?
		bool discrete() const { return _discrete; }
	private:
		bool _discrete;
		std::vector<std::pair<double, double> > _cdf;
	};
}

#endif 

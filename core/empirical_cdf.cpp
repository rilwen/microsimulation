#include "empirical_cdf.hpp"
#include <algorithm>

namespace averisera {
	EmpiricalCDF::EmpiricalCDF(const std::vector<std::pair<double, double> >& cdf, bool discrete)
		: _discrete(discrete), _cdf(cdf)
	{
	}

	// Comparison function used by the search function below
	inline bool cdf_pair_less_than_value(const std::pair<double, double>& cdfpair, double x) {
		return cdfpair.first < x;
	}

	double EmpiricalCDF::prob(double x) const
	{
		// find first pair (x_k, p_k) for which x <= x_k
		const std::vector<std::pair<double, double> >::const_iterator it = std::lower_bound(_cdf.begin(), _cdf.end(), x, cdf_pair_less_than_value);
		if (it == _cdf.end()) {
			// no such x_k ==> return highest possible value for CDF, i.e. 1.0
			return 1.0;
		}
		if (it->first == x) {
			// x == x_k ==> P(X < x) == p_k
			return it->second;
		}
		if (it == _cdf.begin()) {
			// we are below all stored values, so return 0
			return 0.0;
		} else {
			const std::vector<std::pair<double, double> >::const_iterator it2 = it - 1;
			const double p0 = it2->second;
			if (_discrete) {
				return p0;
			} else {
				const double x0 = it2->first;
				const double x1 = it->first;
				const double p1 = it->second;
				const double slope = (p1 - p0) / (x1 - x0);
				return p0 + (x - x0) * slope;
			}
		}
	}
}

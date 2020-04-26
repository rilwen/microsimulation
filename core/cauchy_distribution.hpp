/*
(C) Averisera Ltd 2014-2020
*/
#ifndef __AVERISERA_CAUCHY_DISTRIBUTION_H
#define __AVERISERA_CAUCHY_DISTRIBUTION_H

namespace averisera {
    /** Cauchy distribution */
    class CauchyDistribution {
	public:
        static double cdf(double x);
		static double pdf(double x);
		static double icdf(double p);
	};
}

#endif // __AVERISERA_CAUCHY_DISTRIBUTION_H
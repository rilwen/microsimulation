/*
 * (C) Averisera Ltd 2015
 */
#include "hazard_curve.hpp"
#include <cassert>
#include <cmath>
#include <boost/format.hpp>

namespace averisera {
    namespace microsim {
        HazardCurve::~HazardCurve() {}

        double HazardCurve::jump_probability(double integrated_hazard_rate) {
            assert(integrated_hazard_rate >= 0);
            return - expm1(- integrated_hazard_rate);
        }

        double HazardCurve::integrated_hazard_rate_from_jump_proba(double p) {
            assert(p >= 0);
            assert(p <= 1);
            return - log1p(-p);
        }

		std::unique_ptr<HazardCurve> HazardCurve::multiply_probability(const double t0, const double t1, const double factor) const {
			validate(t0, t1, factor);
			if (factor == 0 || factor == 1) {
				return multiply_hazard_rate(t0, t1, factor);
			}
			if (t0 == t1) {
				return clone();
			}
			const double ihrA = integrated_hazard_rate(t0, t1);
			if (ihrA == 0) {				
				return clone();
			}
			const double pA = jump_probability(ihrA);
			const double pB = factor * jump_probability(t0, t1);
			if (pB > 1) {
				throw std::out_of_range(boost::str(boost::format("HazardCurve: base probability %g too large to apply factor %g") % pA % factor));
			}
			const double ihrB = integrated_hazard_rate_from_jump_proba(pB);
			const double rate_factor = ihrB / ihrA;
			return multiply_hazard_rate(t0, t1, rate_factor);
		}

		void HazardCurve::validate(double t0, double t1, double factor) {
			if (!(factor >= 0)) {
				throw std::out_of_range(boost::str(boost::format("HazardCurve: factor %g negative") % factor));
			}
			if (!(t0 <= t1)) {
				throw std::out_of_range(boost::str(boost::format("HazardCurve: end time %g < start time %g") % t1 % t0));
			}
			if (!(t0 >= 0)) {
				throw std::out_of_range(boost::str(boost::format("HazardCurve: start time %g must be positive or zero") % t0));
			}
		}
    }
}

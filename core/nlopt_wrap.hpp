/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_NLOPT_H
#define __AVERISERA_NLOPT_H

#include <string>

// Wrapper header file turning off certain warnings
// Convenience methods for nlopt may be added here as well.

#ifdef _MSC_VER
#pragma warning(disable : 4267)
#endif // _MSC_VER
#include <nlopt.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4267)
#endif // _MSC_VER

namespace nlopt {
	// Translate nlopt error code into text message.
	const char* retcodestr(result ret);
}

namespace averisera {
	/** Run NLopt optimizer task and handle the error codes */
	nlopt::result run_nlopt(const std::string& problem_name, nlopt::opt& opt, std::vector<double>& x, double& value);	

	/** Stopping conditions consistent with NLopt API.

	Optimiser terminates when the any of the stopping conditions is met.

	Any condition (except for stopval) with a non-positive parameter is ignored.
	Stopval is ignored if it is equal to +HUGE_VAL (default).
	*/
	struct StoppingConditions {
		/// Initialises all parameters except for stopval to zero, and stopval to +HUGE_VAL.
		StoppingConditions();

		/// Stopping condition: f(x) <= stopval.
		double stopval;

		/// Stopping condition: Delta f(x) < ftol_abs.
		double ftol_abs;

		/// Stopping condition: Delta f(x) / |f(x)| < ftol_rel.
		double ftol_rel;

		/// Stopping condition: forall_i Delta x_i < xtol_abs.
		double xtol_abs;

		/// Stopping condition: Delta ||x||_1 / ||x||_1 < xtol_rel.
		double xtol_rel;

		/// Stopping condition: number of function evaluations > maxeval.
		unsigned int maxeval;

		/// Stopping condition: optimisation time in seconds > maxtime;
		double maxtime;
	};
}

#endif

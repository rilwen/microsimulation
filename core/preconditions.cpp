/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include "preconditions.hpp"
#include <numeric>
#include <cmath>

namespace averisera {
	void check_sum(double expected, const std::vector<double>& elements, double tolerance, const char* msg) {
		if (elements.empty()) {
			if (!msg) {
				throw std::domain_error("Testing expected sum against empty vector");
			} else {
				throw std::domain_error(boost::str(boost::format("Testing expected sum against empty vector: %s") % msg));
			}
		}
		const double sum = std::accumulate(elements.begin(), elements.end(), 0.0);
		if (std::abs(expected - sum) > tolerance) {
			if (msg) {
				throw std::domain_error(boost::str(boost::format("Expected sum %g but got %g: %s") % expected % sum % msg));
			} else {
				throw std::domain_error(boost::str(boost::format("Expected sum %g but got %g") % expected % sum));
			}			
		}
	}

	void check_distribution(const std::vector<double>& probs, double tolerance, const char* msg) {
		check_sum(1.0, probs, tolerance, msg);
		check_that(probs.begin(), probs.end(), [](double p){return p >= 0 && p <= 1.0; }, "Probability between 0 and 1");
	}
}

#include "data_exception.hpp"
#include "distribution_empirical.hpp"
#include "log.hpp"
#include "preconditions.hpp"
#include "rng.hpp"
#include "running_statistics.hpp"
#include "segment_search.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>

namespace averisera {
    DistributionEmpirical::DistributionEmpirical(const std::vector<double>& values)
        : _values(values) {
        validate(_values);
        process();
    }

    DistributionEmpirical::DistributionEmpirical(std::vector<double>&& values) {
        validate(values);
		_values = std::move(values);
        process();
    }

    double DistributionEmpirical::draw(RNG& rng) const {
        return _values[static_cast<size_t>(rng.next_uniform(_sample_size - 1))]; // we know that random number will be in [0, _sample_size) range, so we can safely cast to size_t
    }
    
    void DistributionEmpirical::validate(const std::vector<double>& values) {
        if (values.empty()) {
            throw std::domain_error("DistributionEmpirical: no values");
        }
		for (double x : values) {
			if (!std::isfinite(x)) {
				throw DataException("DistributionEmpirical: all sample data must be finite");
			}
		}
    }

    void DistributionEmpirical::process() {
        _sample_size = _values.size();
        std::sort(_values.begin(), _values.end());
		RunningStatistics<double> rs;
		for (double x : _values) {
			assert(std::isfinite(x));
			rs.add(x);
		}
		_mean = rs.mean();
		_variance = rs.variance();
    }

	double DistributionEmpirical::pdf(double x) const {
		// poor man's Dirac delta
		LOG_DEBUG() << "DistributionEmpirical: calculating continuous PDF for discrete distribution";
		if (std::binary_search(_values.begin(), _values.end(), x)) {
			return std::numeric_limits<double>::infinity();
		} else {
			return 0.;
		}
	}

	double DistributionEmpirical::cdf(double x) const {
		const size_t i = SegmentSearch::binary_search_left_inclusive(_values, x);
		if (i == SegmentSearch::NOT_FOUND) {
			return 0;
		} else {
			return static_cast<double>(i + 1) / static_cast<double>(_sample_size);
		}
	}

	double DistributionEmpirical::cdf2(double x) const {
		const size_t i = SegmentSearch::binary_search_right_inclusive(_values, x);
		if (i == SegmentSearch::NOT_FOUND) {
			return 0;
		} else {
			return static_cast<double>(i + 1) / static_cast<double>(_sample_size);
		}		
	}

	double DistributionEmpirical::icdf(const double p) const {
		check_that<std::out_of_range>(p >= 0, "DistributionEmpirical: Probability less than 0");
		check_that<std::out_of_range>(p <= 1, "DistributionEmpirical: Probability more than 1");
		if (p == 0) {
			return infimum();
		} else if (p == 1) {
			return supremum();
		} else {
			const double a = p * static_cast<double>(_sample_size);
			return _values[static_cast<size_t>(std::ceil(a) - 1)];
		}
	}
}

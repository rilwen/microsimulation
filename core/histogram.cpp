// (C) Averisera Ltd 2014-2020
#include "histogram.hpp"
#include <cmath>
#include <stdexcept>
#include <cassert>
#include <algorithm>

namespace averisera {
	Histogram::Histogram(double lower, double upper, size_t n_bins, bool inclusive)
		: _lower(lower), _upper(upper), _n_bins(n_bins), _bin_size((upper - lower) / static_cast<double>(n_bins)), _bins(n_bins)
		, inclusive_(inclusive)
	{
		if (lower > upper) {
			throw std::domain_error("Lower > upper");
		}
		_n_total = 0;
		_n_below = 0;
		_n_above = 0;
		std::fill(_bins.begin(), _bins.end(), 0u);
	}
	
	Histogram::Histogram()
	{
		_lower = 0;
		_upper = 0;
		_n_bins = 0;
		_bin_size = 0;
		_n_total = 0;
		_n_below = 0;
		_n_above = 0;
		inclusive_ = false;
	}
			
	// Add another sample to the histogram
	void Histogram::add(double sample)
	{
		++_n_total;
		if (inclusive_  ? (sample < _lower) :(sample <= _lower)) {
			++_n_below;
		} else if (sample > _upper) {
			++_n_above;
		} else {
			const double r = (sample - _lower) / _bin_size;
			const size_t idx = static_cast<size_t >(ceil(r)) - 1;
			++_bins[std::min(std::max<size_t>(idx, 0), _n_bins - 1)];
		}
	}
	
	// Store histogram probabilities to provided vector, resizing if needed.
	void Histogram::calc_probabilities(std::vector<double>& probs) const
	{
		probs.resize(_n_bins);
		for (unsigned int i = 0; i < _n_bins; ++i) {
			probs[i] = static_cast<double>(_bins[i]) / static_cast<double>(_n_total);
		}
	}
}

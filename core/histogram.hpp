#ifndef __AVERISERA_HISTOGRAM_H
#define __AVERISERA_HISTOGRAM_H

#include <cstdlib>
#include <vector>

namespace averisera {
	// Accumulates samples to form a histogram of a continuous random variable
	class Histogram {
    public:
        /**
		@param lower lower range
        @param upper upper range
        @param n_bins number of bins
		@param inclusive Put samples with value lower in the 1st bin
		*/
        Histogram(double lower, double upper, size_t n_bins, bool inclusive = false);

        // Default constructor
        Histogram();

        // Add another sample to the histogram
        void add(double sample);

        // Return reference to vector with numbers of samples in bins
        const std::vector<size_t>& bins() const { return _bins; }

        // Return number of samples below or equal to lower()
        size_t n_below() const { return _n_below; }

        // Return number of samples above upper()
        size_t n_above() const { return _n_above; }

        // Return total number of samples accumulated
        size_t n_total() const { return _n_total; }

        // Store histogram probabilities to provided vector, resizing if needed.
        void calc_probabilities(std::vector<double>& probs) const;

        // Return estimated probability of finding a value below or equal to lower()
        double calc_prob_below() const { return static_cast<double>(_n_below) / static_cast<double>(_n_total); }

        // Return estimated probability of finding a value above the highest bin
        double calc_prob_above() const { return static_cast<double>(_n_above) / static_cast<double>(_n_total); }

        // Lower bound of the bins
        double lower() const { return _lower; }

        // Upper bound of the bins
        double upper() const { return _upper; }

        // Number of bins
        size_t n_bins() const { return _n_bins; }

        // Bin size == (upper() - lower()) / n_bins()
        double bin_size() const { return _bin_size; }
    private:
        double _lower;
        double _upper;
        size_t _n_bins;
        double _bin_size; // size of the bin
        size_t _n_total;
        std::vector<size_t> _bins;
        size_t _n_below;
        size_t _n_above;
		bool inclusive_;
	};
}

#endif

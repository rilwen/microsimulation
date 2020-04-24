/*
(C) Averisera Ltd 2014
*/
#include <gtest/gtest.h>
#include "core/histogram.hpp"

TEST(Histogram, Test)
{
	const double lower = 0.5;
	const double upper = 10.5;
	const unsigned int n_bins = 10;
		
	averisera::Histogram histogram(lower, upper, n_bins);

	ASSERT_NEAR(1.0, histogram.bin_size(), 1E-15);
	ASSERT_EQ(n_bins, histogram.n_bins());
	ASSERT_EQ(lower, histogram.lower());
	ASSERT_EQ(upper, histogram.upper());

	histogram.add(0);
	histogram.add(11);
	histogram.add(1);
	histogram.add(0.5);
	histogram.add(10.5);
	histogram.add(2.5);
	histogram.add(2);

	ASSERT_EQ(7u, histogram.n_total());
	ASSERT_EQ(2u, histogram.n_below());
	ASSERT_EQ(1u, histogram.n_above());
	ASSERT_NEAR(2.0/7, histogram.calc_prob_below(), 1E-16); // 0 and 0.5
	ASSERT_NEAR(1.0/7, histogram.calc_prob_above(), 1E-16); // 11

	ASSERT_EQ(n_bins, histogram.bins().size());
	ASSERT_EQ(1u, histogram.bins()[0]); // 1
	ASSERT_EQ(2u, histogram.bins()[1]); // 2 and 2.5
	ASSERT_EQ(0u, histogram.bins()[2]);
	ASSERT_EQ(0u, histogram.bins()[3]);
	ASSERT_EQ(0u, histogram.bins()[4]);
	ASSERT_EQ(0u, histogram.bins()[5]);
	ASSERT_EQ(0u, histogram.bins()[6]);
	ASSERT_EQ(0u, histogram.bins()[7]);
	ASSERT_EQ(0u, histogram.bins()[8]);
	ASSERT_EQ(1u, histogram.bins()[9]); // 10.5

	std::vector<double> probs;
	histogram.calc_probabilities(probs);
	ASSERT_EQ(n_bins, probs.size());
	for (size_t i = 0; i < n_bins; ++i) {
		// "<<" generates an error message to be shown if assertion fails
		ASSERT_NEAR(static_cast<double>(histogram.bins()[i]) / 7.0, probs[i], 1E-16) << "Mismatch at position " << i;
	}
}

TEST(Histogram, Inclusive) {
	averisera::Histogram histogram(0, 2, 2, true);
	histogram.add(-1);
	histogram.add(0);
	histogram.add(2);
	histogram.add(3);
	ASSERT_EQ(histogram.n_total(), 4);
	ASSERT_EQ(histogram.n_below(), 1);
	ASSERT_EQ(histogram.n_above(), 1);
}

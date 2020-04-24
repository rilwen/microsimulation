/*
(C) Averisera Ltd 2014
*/
#include <gtest/gtest.h>
#include "core/dirichlet_distribution.hpp"

TEST(DirichletDistribution, Flat) {
	averisera::DirichletDistribution dd(2, 1.0);
	ASSERT_EQ(2u, dd.size());
	ASSERT_EQ(1.0, dd.get_alpha(0));
	ASSERT_EQ(1.0, dd.get_alpha(1));
	std::vector<double> p;
	std::mt19937 rng;
	dd.sample(rng, p);
	ASSERT_EQ(2u, p.size());
	ASSERT_NEAR(1.0, p[0] + p[1], 1E-12);
	ASSERT_TRUE(p[0] >= 0);
	ASSERT_TRUE(p[0] <= 1);
	const size_t n = 20000;
	std::vector<double> samples(n);
	for (size_t i = 0; i < n; ++i) {
		dd.sample(rng, p);
		samples[i] = p[0];
	}
	for (int k = 1; k < 10; ++k) {
		const double max_p = k / 10.0;
		const size_t cnt = std::count_if(samples.begin(), samples.end(), [max_p](double p){return p < max_p; });
		ASSERT_NEAR(max_p, static_cast<double>(cnt) / static_cast<double>(n), 1E-2) << k;
	}
}

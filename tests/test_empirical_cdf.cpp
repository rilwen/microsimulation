// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/empirical_cdf.hpp"

TEST(EmpiricalCDF, Continuous) 
{
	std::vector<std::pair<double, double> > cdf;
	cdf.push_back(std::pair<double, double>(-2, 0.1));
	cdf.push_back(std::pair<double, double>(-1, 0.2));
	cdf.push_back(std::pair<double, double>(0, 0.9));
	cdf.push_back(std::pair<double, double>(2, 1.0));
	averisera::EmpiricalCDF ecdf(cdf, false);
	ASSERT_EQ(0.0, ecdf.prob(-2.1));
	ASSERT_EQ(1.0, ecdf.prob(2.1));
	for (size_t i = 0; i < cdf.size(); ++i) {
		ASSERT_EQ(cdf[i].second, ecdf.prob(cdf[i].first));
	}
	ASSERT_NEAR(0.15, ecdf.prob(-1.5), 1E-15);
}

TEST(EmpiricalCDF, Discrete) 
{
	std::vector<std::pair<double, double> > cdf;
	cdf.push_back(std::pair<double, double>(-2, 0.1));
	cdf.push_back(std::pair<double, double>(-1, 0.2));
	cdf.push_back(std::pair<double, double>(0, 0.9));
	cdf.push_back(std::pair<double, double>(2, 1.0));
	averisera::EmpiricalCDF ecdf(cdf);
	ASSERT_EQ(0.0, ecdf.prob(-2.1));
	ASSERT_EQ(1.0, ecdf.prob(2.1));
	for (size_t i = 0; i < cdf.size(); ++i) {
		ASSERT_EQ(cdf[i].second, ecdf.prob(cdf[i].first));
	}
	ASSERT_NEAR(0.1, ecdf.prob(-1.5), 1E-15);
}


#include <gtest/gtest.h>
#include "core/empirical_cdf_calc.hpp"
#include "core/empirical_cdf.hpp"

TEST(EmpiricalCDFCalculator, Test1)
{
	averisera::EmpiricalCDFCalculator calc(5);
	calc.add(0);
	calc.add(0);
	calc.add(-1);
	calc.add(5);
	calc.add(5);
	calc.add(7); // past original capacity
	std::vector<std::pair<double, double> > cdf;
	calc.calc_cdf(cdf);
	ASSERT_EQ(4u, cdf.size());
	ASSERT_EQ(-1.0, cdf[0].first);
	ASSERT_EQ(0.0, cdf[1].first);
	ASSERT_EQ(5.0, cdf[2].first);
	ASSERT_EQ(7.0, cdf[3].first);
	ASSERT_NEAR(1.0/6, cdf[0].second, 1E-15);
	ASSERT_NEAR(3.0/6, cdf[1].second, 1E-15);
	ASSERT_NEAR(5.0/6, cdf[2].second, 1E-15);
	ASSERT_EQ(1.0, cdf[3].second);

	std::shared_ptr<averisera::EmpiricalCDF> ecdf = calc.calc_cdf();
	for (size_t i = 0; i < 4; ++i) {
		ASSERT_EQ(cdf[i].second, ecdf->prob(cdf[i].first));
	}
}

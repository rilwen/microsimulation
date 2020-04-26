// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include <algorithm>
#include "core/normal_distribution.hpp"

TEST(NormalDistribution,Object) {
	const double mean = 3;
	const double sigma = 0.5;
	averisera::NormalDistribution distr(mean, sigma);
    ASSERT_EQ(mean, distr.mean());
	const double x = 2;
	EXPECT_NEAR(distr.pdf(x), averisera::NormalDistribution::normpdf(x, mean, sigma), 1E-20);
	EXPECT_NEAR(distr.pdf(x), averisera::NormalDistribution::normpdf( (x - mean) / sigma) / sigma, 1E-20);
	EXPECT_NEAR(distr.cdf(x), averisera::NormalDistribution::normcdf( (x - mean) / sigma), 1E-20);
	const double p = distr.cdf(x);
	EXPECT_NEAR(x, distr.icdf(p), 1E-20);
	EXPECT_NEAR(x, mean + sigma * averisera::NormalDistribution::normsinv(p), 1E-20);
}

TEST(NormalDistribution, CDF)
{
	EXPECT_EQ(0.5, averisera::NormalDistribution::normcdf(0));
	EXPECT_EQ(0, averisera::NormalDistribution::normcdf(-std::numeric_limits<double>::max()));
	EXPECT_EQ(1, averisera::NormalDistribution::normcdf(std::numeric_limits<double>::max()));
	const double x0 = -37.53;
	const double x1 = 8.29;
	EXPECT_NE(0, averisera::NormalDistribution::normcdf(x0));
	EXPECT_NE(1, averisera::NormalDistribution::normcdf(x1));
	for (int i = 0; i <= 100; ++i) {
		const double x = x0 + i*(x1 - x0) / 100.0;
		EXPECT_EQ(averisera::NormalDistribution::normcdf(x), 0.5*averisera::NormalDistribution::erfc(-0.7071067811865475244008443621*x));
	}
}

TEST(NormalDistribution, Erfc)
{
	const double tolerance1 = 1E-16;
	EXPECT_NEAR(1.999999999999999999999958162, averisera::NormalDistribution::erfc(-7.0), tolerance1);
	EXPECT_NEAR(1.999999999999999978480263288, averisera::NormalDistribution::erfc(-6.0), tolerance1);
	EXPECT_NEAR(1.999999999998462540205571965, averisera::NormalDistribution::erfc(-5.0), tolerance1);
	EXPECT_NEAR(1.999999984582742099719981148, averisera::NormalDistribution::erfc(-4.0), tolerance1);
	EXPECT_NEAR(1.999977909503001414558627224, averisera::NormalDistribution::erfc(-3.0), tolerance1);
	EXPECT_NEAR(1.995322265018952734162069256, averisera::NormalDistribution::erfc(-2.0), tolerance1);
	EXPECT_NEAR(1.842700792949714869341220635, averisera::NormalDistribution::erfc(-1.0), tolerance1);
	EXPECT_NEAR(1.711155633653515131598937835, averisera::NormalDistribution::erfc(-.75), tolerance1);
	EXPECT_NEAR(1.520499877813046537682746654, averisera::NormalDistribution::erfc(-.50), tolerance1);
	EXPECT_NEAR(1.276326390168236932985068268, averisera::NormalDistribution::erfc(-.25), tolerance1);
	EXPECT_EQ(1, averisera::NormalDistribution::erfc(0));
	EXPECT_EQ(2, averisera::NormalDistribution::erfc(-std::numeric_limits<double>::max()));
	EXPECT_EQ(0, averisera::NormalDistribution::erfc(std::numeric_limits<double>::max()));

	EXPECT_NEAR(0.7236736098317630670149317322, averisera::NormalDistribution::erfc(.25), tolerance1);
	EXPECT_NEAR(0.4795001221869534623172533461, averisera::NormalDistribution::erfc(.50), tolerance1);
	EXPECT_NEAR(0.28884436634648486840106216541, averisera::NormalDistribution::erfc(.75), tolerance1);
	EXPECT_NEAR(0.15729920705028513065877936492, averisera::NormalDistribution::erfc(1.0), tolerance1);
	EXPECT_NEAR(0.0046777349810472658379307436330, averisera::NormalDistribution::erfc(2.0), tolerance1);

	// Back to tolerance 1
	EXPECT_NEAR(0.000022090496998585441372776129583, averisera::NormalDistribution::erfc(3.0), tolerance1);
	EXPECT_NEAR(0.000000015417257900280018852159673486, averisera::NormalDistribution::erfc(4.0), tolerance1);
	EXPECT_NEAR(1.5374597944280348501883434853E-12, averisera::NormalDistribution::erfc(5.0), tolerance1);
	EXPECT_NEAR(2.1519736712498913116593350399E-17, averisera::NormalDistribution::erfc(6.0), tolerance1);

	// Tail
	EXPECT_NEAR(4.1838256077794143986140102238E-23, averisera::NormalDistribution::erfc(7.0), 1E-30);
	EXPECT_NEAR(1.12242971729829270799678884432E-29, averisera::NormalDistribution::erfc(8.0), 1E-40);
	EXPECT_NEAR(4.1370317465138102380539034672E-37, averisera::NormalDistribution::erfc(9.0), 1E-50);
	EXPECT_NEAR(2.0884875837625447570007862949E-45, averisera::NormalDistribution::erfc(10.0), 1E-60);

	EXPECT_NE(2, averisera::NormalDistribution::erfc(-5.86));
	EXPECT_NE(0, averisera::NormalDistribution::erfc(26.54));
}

TEST(NormalDistribution, Erf)
{
	const double tolerance1 = 2E-16;

	EXPECT_NEAR(-0.9999999999999999999999999999887757028270, averisera::NormalDistribution::erf(-8.0), tolerance1);
	EXPECT_NEAR(-0.9999999999999999999999581617439222058560, averisera::NormalDistribution::erf(-7.0), tolerance1);
	EXPECT_NEAR(-0.9999999999999999784802632875010868834066, averisera::NormalDistribution::erf(-6.0), tolerance1);
	EXPECT_NEAR(-0.9999999999984625402055719651498116565146, averisera::NormalDistribution::erf(-5.0), tolerance1);
	EXPECT_NEAR(-0.99999998458274209971998114784032651332, averisera::NormalDistribution::erf(-4.0), tolerance1);
	EXPECT_NEAR(-0.99997790950300141455862722387041706621, averisera::NormalDistribution::erf(-3.0), tolerance1);
	EXPECT_NEAR(-0.99532226501895273416206925636704571137, averisera::NormalDistribution::erf(-2.0), tolerance1);

	EXPECT_NEAR(-0.84270079294971486934122063508, averisera::NormalDistribution::erf(-1.0), 2E-16);
	EXPECT_NEAR(-0.71115563365351513159893783458, averisera::NormalDistribution::erf(-.75), tolerance1);
	EXPECT_NEAR(-0.52049987781304653768274665390, averisera::NormalDistribution::erf(-.50), tolerance1);
	EXPECT_NEAR(-0.27632639016823693298506826776, averisera::NormalDistribution::erf(-.25), 1E-16);
	EXPECT_NEAR(0.84270079294971486934122063508, averisera::NormalDistribution::erf(1.0), 2E-16);
	EXPECT_NEAR(0.71115563365351513159893783458, averisera::NormalDistribution::erf(.75), tolerance1);
	EXPECT_NEAR(0.52049987781304653768274665390, averisera::NormalDistribution::erf(.50), tolerance1);
	EXPECT_NEAR(0.27632639016823693298506826776, averisera::NormalDistribution::erf(.25), 1E-16);

	EXPECT_NEAR(0.9999999999999999999999999999887757028270, averisera::NormalDistribution::erf(8.0), tolerance1);
	EXPECT_NEAR(0.9999999999999999999999581617439222058560, averisera::NormalDistribution::erf(7.0), tolerance1);
	EXPECT_NEAR(0.9999999999999999784802632875010868834066, averisera::NormalDistribution::erf(6.0), tolerance1);
	EXPECT_NEAR(0.9999999999984625402055719651498116565146, averisera::NormalDistribution::erf(5.0), tolerance1);
	EXPECT_NEAR(0.99999998458274209971998114784032651332, averisera::NormalDistribution::erf(4.0), tolerance1);
	EXPECT_NEAR(0.99997790950300141455862722387041706621, averisera::NormalDistribution::erf(3.0), tolerance1);
	EXPECT_NEAR(0.99532226501895273416206925636704571137, averisera::NormalDistribution::erf(2.0), tolerance1);


	EXPECT_EQ(0, averisera::NormalDistribution::erf(0));
	EXPECT_EQ(-1, averisera::NormalDistribution::erf(-std::numeric_limits<double>::max()));
	EXPECT_EQ(1, averisera::NormalDistribution::erf(std::numeric_limits<double>::max()));
}

TEST(NormalDistribution, NormsInv)
{
	EXPECT_EQ(-std::numeric_limits<double>::infinity(), averisera::NormalDistribution::normsinv(0));
	EXPECT_EQ(std::numeric_limits<double>::infinity(), averisera::NormalDistribution::normsinv(1));
	EXPECT_EQ(0, averisera::NormalDistribution::normsinv(0.5));
	const double tol = 1E-15;
	std::vector<double> xs;
	xs.push_back(-6);
	xs.push_back(-4.5);
	xs.push_back(-1);
	xs.push_back(0);
	xs.push_back(1);
	xs.push_back(4.5);
	xs.push_back(6);
	for (std::vector<double>::const_iterator i = xs.begin(); i != xs.end(); ++i) {
		const double p = averisera::NormalDistribution::normcdf(*i);
		const double x = averisera::NormalDistribution::normsinv(p);
		const double diff = std::min( std::abs(*i - x), std::abs(p - averisera::NormalDistribution::normcdf(x)));
		EXPECT_NEAR(0, diff, tol);
	}
}

TEST(NormalDistribution, ConditionalMean) {
    averisera::NormalDistribution d(2.1, 1.4);
    const double from_super = d.Distribution::conditional_mean(1, 4);
    const double from_nd = d.conditional_mean(1, 4);
    ASSERT_NEAR(from_super, from_nd, 1E-14);
}

TEST(NormalDistribution, Estimate) {
	const std::vector<double> sample({ -1, -std::numeric_limits<double>::infinity(), 1, std::numeric_limits<double>::quiet_NaN() });
	averisera::NormalDistribution nd_biased(averisera::NormalDistribution::estimate(sample.begin(), sample.end(), false));
	ASSERT_NEAR(0.0, nd_biased.mean(), 1E-16);
	ASSERT_NEAR(1.0, nd_biased.sigma(), 4E-16);
	averisera::NormalDistribution nd_unbiased(averisera::NormalDistribution::estimate(sample.begin(), sample.end(), true));
	ASSERT_NEAR(0.0, nd_unbiased.mean(), 1E-16);
	ASSERT_NEAR(sqrt(2.0), nd_unbiased.sigma(), 5E-16);
}
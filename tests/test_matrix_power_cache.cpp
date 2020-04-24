/*
(C) Averisera Ltd 2015
*/
#include <gtest/gtest.h>
#include "core/matrix_power_cache.hpp"
#include <Eigen/Core>

TEST(MatrixPowerCache, Test) {
	Eigen::MatrixXd m(2, 2);
	m << 1, -0.5,
		2, 0.5;
	averisera::MatrixPowerCache<Eigen::MatrixXd> cache(m);
	ASSERT_EQ(cache.max_power(), 1u);
	ASSERT_EQ((cache.power(1) - m).norm(), 0.);
	ASSERT_TRUE(cache.is_power_available(1));
	ASSERT_FALSE(cache.is_power_available(2));
	ASSERT_FALSE(cache.is_power_available(2000));
	cache.power(4);
	ASSERT_EQ(cache.max_power(), 4u);
	ASSERT_TRUE(cache.is_power_available(2));
	ASSERT_FALSE(cache.is_power_available(3));
	ASSERT_TRUE(cache.is_power_available(4));	
	ASSERT_FALSE(cache.is_power_available(2000));
	ASSERT_EQ((cache.power(1) - m).norm(), 0.);
	ASSERT_NEAR((cache.power(2) - m*m).norm(), 0, 1E-12);
	ASSERT_NEAR((cache.power(4) - cache.power(2)*cache.power(2)).norm(), 0, 1E-12);
	ASSERT_NEAR((cache.power(3) - m*cache.power(2)).norm(), 0, 1E-12);
	ASSERT_TRUE(cache.is_power_available(3));
}

/*
(C) Averisera Ltd 2017
*/

#include <gtest/gtest.h>
#include "core/interpolator_impl_piecewise_polynomial.hpp"

using namespace averisera;

TEST(InterpolatorImplPiecewisePolynomial, Default) {
	const InterpolatorImplPiecewisePolynomial<2> interp;
	ASSERT_EQ(0.0, interp.lowerBound());
	ASSERT_EQ(1.0, interp.upperBound());
	ASSERT_EQ(0.0, interp.evaluate(0.0));
	ASSERT_EQ(0.0, interp.evaluate(0.5));
	ASSERT_EQ(0.0, interp.evaluate(1.0));
}

#include <gtest/gtest.h>
#include "core/math_utils.hpp"
#include "testing/assertions.hpp"
#include "testing/rng_precalc.hpp"

using namespace averisera;
using namespace averisera::testing;

TEST(MathUtils, Sgn) {
	ASSERT_EQ(-1, MathUtils::sgn(-10));
	ASSERT_EQ(0, MathUtils::sgn(0));
	ASSERT_EQ(1, MathUtils::sgn(5));
}

TEST(MathUtils, Pow) {
	ASSERT_EQ(9u, MathUtils::pow(3, 2));
	ASSERT_EQ(8u, MathUtils::pow(2, 3));
}

TEST(MathUtils, RealProbConversions) {
	ASSERT_EQ(1.0, MathUtils::logit_to_prob(std::numeric_limits<double>::infinity()));
	ASSERT_EQ(std::numeric_limits<double>::infinity(), MathUtils::prob_to_logit(1.0));
	ASSERT_EQ(0.0, MathUtils::logit_to_prob(-std::numeric_limits<double>::infinity()));
	ASSERT_EQ(-std::numeric_limits<double>::infinity(), MathUtils::prob_to_logit(0.0));
	ASSERT_NEAR(0.5, MathUtils::logit_to_prob(0.0), 1E-20);
	ASSERT_NEAR(0.0, MathUtils::prob_to_logit(0.5), 1E-20);
	for (unsigned int k = 1; k <= 100; ++k) {
		const double x0 = MathUtils::prob_to_logit((k - 1) * 0.01);
		const double x1 = MathUtils::prob_to_logit(k * 0.01);
		ASSERT_LT(x0, x1) << k;
		ASSERT_LT(MathUtils::logit_to_prob(x0), MathUtils::logit_to_prob(x1)) << k;
	}
	ASSERT_LT(MathUtils::prob_to_logit(0.0), MathUtils::prob_to_logit(1E-20));
	ASSERT_LT(MathUtils::prob_to_logit(1.0 - 1E-15), MathUtils::prob_to_logit(1.0));
	const double ps[] = { 0.0, 1E-20, 0.1, 0.5, 0.9, 1.0 - 1E-15, 1.0 };
	for (double p : ps) {
		const double x = MathUtils::prob_to_logit(p);
		const double p2 = MathUtils::logit_to_prob(x);
		ASSERT_LE(p2, 1.0) << p;
		ASSERT_GE(p2, 0.0) << p;
		ASSERT_NEAR(p, p2, 2E-16) << p;
	}
}

TEST(MathUtils, LogitToProbDerivative) {
	const double x = 1;
	const double actual = MathUtils::logit_to_prob_derivative(x);
	const double eps = 1e-6;
	const double expected = (MathUtils::logit_to_prob(x + eps) - MathUtils::logit_to_prob(x - eps)) / (2 * eps);
	ASSERT_NEAR(expected, actual, eps);
	ASSERT_EQ(0., MathUtils::logit_to_prob_derivative(std::numeric_limits<double>::infinity()));
	ASSERT_EQ(0., MathUtils::logit_to_prob_derivative(-std::numeric_limits<double>::infinity()));
}

TEST(MathUtils, CumProd) {
    const std::vector<double> x = {0.3, 1.4, -0.3};
    ASSERT_NEAR(0.3 * 1.4 * (-0.3), MathUtils::cum_prod(x.begin(), x.end(), 1.0), 1E-15);
}

TEST(MathUtils, SafeCastFromFloat) {
    ASSERT_EQ(10, MathUtils::safe_cast<int>(10.12));
    ASSERT_EQ(-10, MathUtils::safe_cast<int>(-10.12));
    ASSERT_THROW(MathUtils::safe_cast<int>(1E100), std::out_of_range);
    ASSERT_THROW(MathUtils::safe_cast<int>(-1E100), std::out_of_range);
    ASSERT_THROW(MathUtils::safe_cast<unsigned int>(-100.0), std::out_of_range);
}

TEST(MathUtils, SafeCastFromIntToIntSameSign) {
    ASSERT_EQ(10, MathUtils::safe_cast<int>(static_cast<char>(10)));
    ASSERT_EQ(-10, MathUtils::safe_cast<char>(static_cast<int>(-10)));
    ASSERT_THROW(MathUtils::safe_cast<char>(12000), std::out_of_range);
    ASSERT_THROW(MathUtils::safe_cast<char>(-12000), std::out_of_range);
}

TEST(MathUtils, SafeCastFromIntToIntSignedToUnsigned) {
    ASSERT_EQ(10u, MathUtils::safe_cast<unsigned int>(static_cast<char>(10)));
    ASSERT_EQ(11, MathUtils::safe_cast<unsigned char>(static_cast<int>(11)));
    ASSERT_THROW(MathUtils::safe_cast<unsigned int>(-12000), std::out_of_range);
    ASSERT_THROW(MathUtils::safe_cast<unsigned char>(12000), std::out_of_range);
}

TEST(MathUtils, SafeCastFromIntToIntUnsignedToSigned) {
    ASSERT_EQ(10, MathUtils::safe_cast<int>(static_cast<unsigned char>(10)));
    ASSERT_EQ(11, MathUtils::safe_cast<char>(static_cast<unsigned int>(11)));
    ASSERT_THROW(MathUtils::safe_cast<char>(static_cast<unsigned char>(200)), std::out_of_range);
}

TEST(MathUtils, SafeCastFromIntToFloat) {
    ASSERT_EQ(10., MathUtils::safe_cast<double>(static_cast<char>(10)));
    ASSERT_EQ(-10.0, MathUtils::safe_cast<double>(static_cast<int>(-10)));
}

TEST(MathUtils, SafeCastSameType) {
    ASSERT_EQ(10, MathUtils::safe_cast<char>(static_cast<char>(10)));
    ASSERT_EQ(-10, MathUtils::safe_cast<int>(static_cast<int>(-10)));
}

TEST(MathUtils, RandomRound) {
	RNGPrecalc rng({ 0.1, 0.01, 0.5 });
	const double x = 1.02;
	ASSERT_EQ(1.0, MathUtils::random_round(x, rng));
	ASSERT_EQ(2.0, MathUtils::random_round(x, rng));
	ASSERT_EQ(1.0, MathUtils::random_round(x, rng));
}

TEST(MathUtils, SolveQuadratic) {
	double x1, x2;
	ASSERT_EQ(1, MathUtils::solve_quadratic(0, 0.5, 1, x1, x2));
	ASSERT_NEAR(-2, x1, 1E-14);
	ASSERT_EQ(0, MathUtils::solve_quadratic(0, 0, 0.5, x1, x2));
	ASSERT_EQ(0, MathUtils::solve_quadratic(2, 0, 0.5, x1, x2));
	ASSERT_EQ(1, MathUtils::solve_quadratic(1, -2, 1, x1, x2));
	ASSERT_NEAR(1, x1, 1E-15);
	ASSERT_EQ(2, MathUtils::solve_quadratic(2, 0, -0.5, x1, x2));
	ASSERT_NEAR(0.5, x1, 1E-15) << x1;
	ASSERT_NEAR(-0.5, x2, 1E-15) << x2;
	ASSERT_EQ(2, MathUtils::solve_quadratic(2E30, 0, -0.5E30, x1, x2));
	ASSERT_NEAR(0.5, x1, 1E-15) << x1;
	ASSERT_NEAR(-0.5, x2, 1E-15) << x2;
}

TEST(MathUtils, min_element_randomised) {
	RNGPrecalc rng({ 0.1, 0.9 });
	const std::vector<int> sample({ 0, -1, -2, 10, -2, 3 });
	auto it = MathUtils::min_element_randomised(sample.begin(), sample.end(), rng); // uses u = 0.1
	ASSERT_EQ(4, std::distance(sample.begin(), it));
	it = MathUtils::min_element_randomised(sample.begin(), sample.end(), rng); // uses u = 0.9
	ASSERT_EQ(2, std::distance(sample.begin(), it));
}

TEST(MathUtils, max_element_randomised) {
	RNGPrecalc rng({ 0.1, 0.9 });
	const std::vector<int> sample({ 0, 1, 2, -10, 2, -3 });
	auto it = MathUtils::max_element_randomised(sample.begin(), sample.end(), rng); // uses u = 0.1
	ASSERT_EQ(4, std::distance(sample.begin(), it));
	it = MathUtils::max_element_randomised(sample.begin(), sample.end(), rng); // uses u = 0.9
	ASSERT_EQ(2, std::distance(sample.begin(), it));
}

TEST(MathUtils, probabilities_to_relative_logits) {
	std::vector<double> probs({ 0.5, 0.5 });
	std::vector<double> rel_logits(1);
	MathUtils::probabilities_to_relative_logits(probs.begin(), probs.end(), rel_logits.begin());
	ASSERT_ALL_NEAR(std::vector<double>({ 0 }), rel_logits, 1e-15, "Equal probabilities");
	probs = { 0.2, 0.8 };
	MathUtils::probabilities_to_relative_logits(probs.begin(), probs.end(), rel_logits.begin());
	ASSERT_ALL_NEAR(std::vector<double>({ log(4) }), rel_logits, 1e-15, "Not equal probabilities");
	probs = { 0., 1. };
	ASSERT_THROW(MathUtils::probabilities_to_relative_logits(probs.begin(), probs.end(), rel_logits.begin()), std::domain_error);
	probs = { 1., 0. };
	ASSERT_THROW(MathUtils::probabilities_to_relative_logits(probs.begin(), probs.end(), rel_logits.begin()), std::domain_error);
	probs.resize(0);
	ASSERT_THROW(MathUtils::probabilities_to_relative_logits(probs.begin(), probs.end(), rel_logits.begin()), std::invalid_argument);
}

TEST(MathUtils, relative_logits_to_probabilities) {
	std::vector<double> rel_logits({ 1, 0 });
	std::vector<double> probs(3);
	MathUtils::relative_logits_to_probabilities(rel_logits.begin(), rel_logits.end(), probs.begin());
	ASSERT_ALL_NEAR(std::vector<double>({
		1 / (1 + 2 * exp(1)),
		exp(1) / (1 + 2 * exp(1)),
		exp(1) / (1 + 2 * exp(1))
	}), probs, 1e-15, "Not zero logits");
	rel_logits = { 0, 0 };
	MathUtils::relative_logits_to_probabilities(rel_logits.begin(), rel_logits.end(), probs.begin());
	ASSERT_ALL_NEAR(std::vector<double>({
		1 / 3.,
		1 / 3.,
		1 / 3.
	}), probs, 1e-15, "Zero logits");
	rel_logits.resize(0);
	probs.resize(1);
	MathUtils::relative_logits_to_probabilities(rel_logits.begin(), rel_logits.end(), probs.begin());
	ASSERT_EQ(1, probs[0]);
}
#include <gtest/gtest.h>
#include "core/ssq_divergence.hpp"

TEST(SSQDivergence, SingleSample) {
	Eigen::MatrixXd p(2, 1);
	p(0, 0) = 1;
	p(1, 0) = 0;
	Eigen::VectorXd nbr_surveys(1);
	nbr_surveys[0] = 1;
	averisera::SSQDivergence ssq(p, nbr_surveys);
	ASSERT_EQ(2u, ssq.dim());
	const Eigen::MatrixXd& weight = ssq.weight(0);
	ASSERT_EQ(1u, ssq.weights().size());
	ASSERT_EQ(weight, ssq.weights()[0]);
	Eigen::Vector2d pert_p(0.99, 0.01);
	const auto diff = pert_p - p.col(0);
	const double error = diff.dot(weight * diff);
	ASSERT_TRUE(error > 0);
}

TEST(SSQDivergence, ProbOne) {
	Eigen::MatrixXd p(2, 1);
	p(0, 0) = 1;
	p(1, 0) = 0;
	Eigen::VectorXd nbr_surveys(1);
	nbr_surveys[0] = 1000;
	averisera::SSQDivergence ssq(p, nbr_surveys);
	const Eigen::MatrixXd& weight = ssq.weight(0);
	Eigen::Vector2d pert_p(0.99, 0.01);
	const auto diff = pert_p - p.col(0);
	const double error = diff.dot(weight * diff);
	ASSERT_TRUE(error > 0);

	nbr_surveys[0] *= 2;
	averisera::SSQDivergence ssq100(p, nbr_surveys);
	const Eigen::MatrixXd& weight100 = ssq100.weight(0);
	const double error100 = diff.dot(weight100 * diff);
	ASSERT_TRUE(error100 > 0);
	ASSERT_NEAR(4, error100 / error, 3E-2);
}

TEST(SSQDivergence, FifyFifty) {
	Eigen::MatrixXd p(2, 1);
	p(0, 0) = 0.5;
	p(1, 0) = 0.5;
	Eigen::VectorXd nbr_surveys(1);
	nbr_surveys[0] = 1000;
	averisera::SSQDivergence ssq(p, nbr_surveys);
	const Eigen::MatrixXd& weight = ssq.weight(0);
	Eigen::Vector2d pert_p(0.55, 0.45);
	const auto diff = pert_p - p.col(0);
	const double error = diff.dot(weight * diff);
	ASSERT_TRUE(error > 0);

	nbr_surveys[0] *= 2;
	averisera::SSQDivergence ssq100(p, nbr_surveys);
	const Eigen::MatrixXd& weight100 = ssq100.weight(0);
	const double error100 = diff.dot(weight100 * diff);
	ASSERT_TRUE(error100 > 0);
	ASSERT_NEAR(2, error100 / error, 1E-2);
}

TEST(SSQDivergence, Multiple) {
	Eigen::MatrixXd p(2, 2);
	p(0, 0) = 0.8;
	p(1, 0) = 0.2;
	p(0, 1) = 0.7;
	p(1, 1) = 0.3;
	Eigen::VectorXd nbr_surveys(2);
	nbr_surveys[0] = 100;
	nbr_surveys[1] = 120;
	averisera::SSQDivergence ssq(p, nbr_surveys);
	ASSERT_EQ(2u, ssq.weights().size());
	for (unsigned int i = 0; i < 2; ++i) {
		Eigen::VectorXd ns(1);
		ns[0] = nbr_surveys[i];
		averisera::SSQDivergence ssq_i(Eigen::MatrixXd(p.col(i)), ns);
		ASSERT_EQ(ssq.weight(i), ssq_i.weight(0)) << i;
	}
}

TEST(SSQDivergence, NoSurveys) {
	Eigen::MatrixXd p(2, 1);
	p(0, 0) = 0.8;
	p(1, 0) = 0.2;
	Eigen::VectorXd nbr_surveys(1);
	nbr_surveys[0] = 0;
	averisera::SSQDivergence ssq(p, nbr_surveys);
	ASSERT_EQ(1u, ssq.weights().size());
	ASSERT_TRUE(ssq.weights()[0].norm() > 0);
}


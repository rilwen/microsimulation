/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include <gtest/gtest.h>
#include "core/mlr.hpp"
#include "core/ssq_divergence.hpp"
#include "core/observed_discrete_data.hpp"
#include "core/statistics.hpp"
#include <numeric>
#include <Eigen/Dense>

TEST(MultinomialLogisticRegression, Test) {
	const unsigned int dim = 3;
	ASSERT_EQ(4u, averisera::MultinomialLogisticRegression::dof(dim));
	averisera::MultinomialLogisticRegression mlr(dim);
	ASSERT_EQ(dim, mlr.dim());
	ASSERT_EQ(dim - 1, mlr.a().size());
	ASSERT_EQ(dim - 1, mlr.b().size());
	const double a0 = 0.1;
	const double a1 = -0.12;
	const double b0 = -0.25;
	const double b1 = 0.35;
	mlr.a()[0] = a0;
	mlr.a()[1] = a1;
	mlr.b()[0] = b0;
	mlr.b()[1] = b1;
	double sump = 0;
	const double t = 0.4;
	std::vector<double> probs(dim);	
	std::vector<double> probs2(dim);
	mlr.p(t, probs2);
	for (unsigned int i = 0; i < dim; ++i) {
		const double p = mlr.p(t, i);
		probs[i] = p;
		ASSERT_NEAR(p, probs2[i], 1E-20);
		ASSERT_TRUE(p >= 0);
		ASSERT_TRUE(p <= 1);
		sump += p;
		const double ln_p = mlr.ln_p(t, i);
		ASSERT_NEAR(log(p), ln_p, 1E-20);
	}
	ASSERT_NEAR(1.0, sump, 1E-15);

	ASSERT_NEAR(1.0 / (1.0 + exp(a0 + b0 * t) + exp(a1 + b1*t)), probs2[0], 1E-16);
	ASSERT_NEAR(exp(a0 + b0*t) / (1.0 + exp(a0 + b0 * t) + exp(a1 + b1*t)), probs2[1], 1E-16);
	ASSERT_NEAR(exp(a1 + b1*t) / (1.0 + exp(a0 + b0 * t) + exp(a1 + b1*t)), probs2[2], 1E-16);

	//const double p0 = probs2[0];
	const double ew0 = exp(a0 + b0*t);
	const double ew1 = exp(a1 + b1*t);
	// dp[0]/da[k]
	ASSERT_NEAR(- ew0 / pow(1.0 + ew0 + ew1, 2), mlr.dp_da(probs2, probs2[0], 0, 0), 1E-16);
	ASSERT_NEAR(-ew1 / pow(1.0 + ew0 + ew1, 2), mlr.dp_da(probs2, probs2[0], 0, 1), 1E-16);
	// dp[1]/da[k]
	ASSERT_NEAR(ew0 / (1.0 + ew0 + ew1) - ew0 * ew0 / pow(1.0 + ew0 + ew1, 2), mlr.dp_da(probs2, probs2[1], 1, 0), 1E-16);
	ASSERT_NEAR(-ew0 * ew1 / pow(1.0 + ew0 + ew1, 2), mlr.dp_da(probs2, probs2[1], 1, 1), 1E-16);
	// dp[2]/da[k]
	ASSERT_NEAR(-ew0 * ew1 / pow(1.0 + ew0 + ew1, 2), mlr.dp_da(probs2, probs2[2], 2, 0), 1E-16);
	ASSERT_NEAR(ew1 / (1.0 + ew0 + ew1) - ew1 * ew1 / pow(1.0 + ew0 + ew1, 2), mlr.dp_da(probs2, probs2[2], 2, 1), 1E-16);

	for (unsigned int j = 0; j < dim - 1; ++j) {
		double sum = 0;
		for (unsigned int k = 0; k < dim; ++k) {
			sum += mlr.dp_da(probs2, probs2[k], k, j);
		}
		ASSERT_NEAR(0.0, sum, 1E-14) << j;
	}
	
	std::vector<double> grad(2 * (dim - 1));
	const double ns = 10;
	// Test at maximum
	mlr.log_likelihood_grad(t, probs2, ns, grad);
	EXPECT_NEAR(0.0, grad[0], 1E-14);
	EXPECT_NEAR(0.0, grad[dim - 1], 1E-14);
	EXPECT_NEAR(0.0, grad[1], 1E-14);
	EXPECT_NEAR(0.0, grad[dim], 1E-14);
	// Test away from maximum
	probs[0] = 0.1;
	probs[1] = 0.1;
	probs[2] = 0.8;
	mlr.log_likelihood_grad(t, probs, ns, grad);
	for (unsigned int j = 0; j < dim - 1; ++j) {
		double expected = 0;
		for (unsigned int k = 0; k < dim; ++k) {
			expected += probs[k] * mlr.dp_da(probs2, probs2[k], k, j) / probs2[k];
		}
		expected *= ns;
		EXPECT_NEAR(expected, grad[j], 1E-14) << j;
		EXPECT_NEAR(t * expected, grad[j + dim - 1], 1E-14) << j;
	}
}

TEST(MultinomialLogisticRegression, LargeAB) {
	const unsigned int dim = 2;
	averisera::MultinomialLogisticRegression mlr(dim);
	mlr.a()[0] = 500;
	mlr.b()[0] = -500;
	std::vector<double> p(dim);
	double t = 1;
	mlr.p(t, p);
	ASSERT_NEAR(0.5, p[0], 4E-16);
	ASSERT_NEAR(0.5, p[1], 4E-16);
	t = 2;
	mlr.p(t, p);
	ASSERT_NEAR(1., p[0], 4E-16);
	ASSERT_NEAR(0., p[1], 4E-16);
	t = -2;
	mlr.p(t, p);
	ASSERT_NEAR(0., p[0], 4E-16);
	ASSERT_NEAR(1., p[1], 4E-16);	
}

TEST(MultinomialLogisticRegression, SSQ) {
	const unsigned int dim = 3;
	averisera::MultinomialLogisticRegression mlr(dim);
	mlr.a()[0] = 0.1;
	mlr.a()[1] = -0.1;
	mlr.b()[0] = -0.2;
	mlr.b()[1] = 0.2;
	const std::vector<double> times = { -1, 1 };
	const size_t T = times.size();
	Eigen::MatrixXd p(dim, T);
	for (size_t k = 0; k < dim; ++k) {
		for (size_t l = 0; l < T; ++l) {
			p(k, l) = mlr.p(times[l], static_cast<unsigned int>(k));
		}
	}
	std::vector<double> work(dim);
	Eigen::MatrixXd weights(dim, dim);
	weights << 1, 0.1, 0.2,
		0.1, 1.1, 0.05,
		0.2, 0.05, 0.95;
	for (size_t t = 0; t < T; ++t) {
		ASSERT_NEAR(0.0, mlr.ssq(times[t], p.col(t), weights, work), 1E-24) << t;
	}
	const double eps = 0.1;
	for (unsigned int k = 0; k < dim; ++k) {
		for (unsigned int l = 0; l < T; ++l) {
			p(k, l) += eps;
		}
	}
	const double sumw = weights.array().sum();
	for (unsigned int t = 0; t < T; ++t) {
		const double actual = mlr.ssq(times[t], p.col(t), weights, work);
		const double expected = eps * eps * sumw;
		ASSERT_NEAR(expected, actual, 1E-16) << t;
	}
}

TEST(MultinomialLogisticRegression, SSQGrad) {
	const unsigned int dim = 3;
	averisera::MultinomialLogisticRegression mlr(dim);
	mlr.a()[0] = 0.1;
	mlr.a()[1] = -0.1;
	mlr.b()[0] = -0.2;
	mlr.b()[1] = 0.2;
	const double t = 1.5;
	Eigen::VectorXd p(dim);
	std::vector<double> grad(2 * (dim - 1));
	for (unsigned int k = 0; k < dim; ++k) {
			p[k] = mlr.p(t, k);
	}
	std::vector<double> work(dim);
	Eigen::MatrixXd weights(dim, dim);
	weights << 1, 0.1, 0.2,
		0.1, 1.1, 0.05,
		0.2, 0.05, 0.95;
	ASSERT_NEAR(0.0, mlr.ssq_grad(t, p, weights, grad, work), 1E-24);	

	const double eps = 1E-8;
	for (unsigned int j = 0; j < dim - 1; ++j) {
		const double orig_a = mlr.a()[j];
		mlr.a()[j] = orig_a + eps/2;
		double ssq_hi = mlr.ssq(t, p, weights, work);
		mlr.a()[j] = orig_a - eps/2;
		double ssq_lo = mlr.ssq(t, p, weights, work);
		ASSERT_NEAR((ssq_hi - ssq_lo) / eps, grad[j], eps) << j;
		mlr.a()[j] = orig_a;

		const double orig_b = mlr.b()[j];
		mlr.b()[j] = orig_b + eps/2;
		ssq_hi = mlr.ssq(t, p, weights, work);
		mlr.b()[j] = orig_b - eps / 2;
		ssq_lo = mlr.ssq(t, p, weights, work);
		ASSERT_NEAR((ssq_hi - ssq_lo) / eps, grad[dim - 1 + j], eps) << j;
		mlr.b()[j] = orig_b;
	}

	for (unsigned int k = 0; k < dim; ++k) {
			p[k] += 0.1;
	}
	const double sumw = weights.array().sum();
	ASSERT_NEAR(0.01 * sumw, mlr.ssq_grad(t, p, weights, grad, work), 1E-16);
}

TEST(MultinomialLogisticRegression, LogLikelihoodGrad) {
	const unsigned int dim = 3;
	averisera::MultinomialLogisticRegression mlr(dim);
	mlr.a()[0] = 0.1;
	mlr.a()[1] = -0.11;
	mlr.b()[0] = -0.21;
	mlr.b()[1] = 0.22;
	const double t = 0.1;
	Eigen::VectorXd p(dim);
	std::vector<double> grad(2 * (dim - 1));
	double expected_ll = 0.0;
	const double ns = 100;
	for (unsigned int k = 0; k < dim; ++k) {
		p[k] = mlr.p(t, k);
		expected_ll += ns * p[k] * log(p[k]);
	}
	const double ll = mlr.log_likelihood(t, p, ns);
	ASSERT_NEAR(expected_ll, ll, 1E-13);
	ASSERT_NEAR(ll, mlr.log_likelihood_grad(t, p, ns, grad), 1E-16);

	// Test the gradient away from maximum
	p[0] = 0.2;
	p[1] = 0.5;
	p[2] = 0.3;
	mlr.log_likelihood_grad(t, p, ns, grad);
	const double eps = 1E-8;
	const double tol = 1E-4;
	for (unsigned int j = 0; j < dim - 1; ++j) {
		const double orig_a = mlr.a()[j];
		mlr.a()[j] = orig_a + eps / 2;
		double ll_hi = mlr.log_likelihood(t, p, ns);
		mlr.a()[j] = orig_a - eps / 2;
		double ll_lo = mlr.log_likelihood(t, p, ns);
		ASSERT_NEAR((ll_hi - ll_lo) / eps, grad[j], std::abs(grad[j]) * tol) << j;
		mlr.a()[j] = orig_a;

		const double orig_b = mlr.b()[j];
		mlr.b()[j] = orig_b + eps / 2;
		ll_hi = mlr.log_likelihood(t, p, ns);
		mlr.b()[j] = orig_b - eps / 2;
		ll_lo = mlr.log_likelihood(t, p, ns);
		ASSERT_NEAR((ll_hi - ll_lo) / eps, grad[dim - 1 + j], std::abs(grad[dim - 1 + j]) * tol) << j;
		mlr.b()[j] = orig_b;
	}

	const double ll2 = mlr.log_likelihood(t, p, ns);
	for (unsigned int k = 0; k < dim; ++k) {
		p[k] *= 2;
	}
	ASSERT_NEAR(2*ll2, mlr.log_likelihood(t, p, ns), 1E-16);
}

TEST(MultinomialLogisticRegression, DpDa) {
	const unsigned int dim = 3;
	averisera::MultinomialLogisticRegression mlr(dim);
	mlr.a()[0] = 0.1;
	mlr.a()[1] = -0.11;
	mlr.b()[0] = -0.21;
	mlr.b()[1] = 0.22;
	const double t = 0.1;
	Eigen::VectorXd p(dim);
	std::vector<double> grad(2 * (dim - 1));
	Eigen::MatrixXd dpda(dim, dim - 1);
	for (unsigned int k = 0; k < dim; ++k) {
		p[k] = mlr.p(t, k);		
	}
ASSERT_NEAR(1.0, p.sum(), 1E-14);
for (unsigned int k = 0; k < dim; ++k) {
	for (unsigned int l = 0; l < dim - 1; ++l) {
		dpda(k, l) = mlr.dp_da(p, p[k], k, l);
	}
}
for (unsigned int l = 0; l < dim - 1; ++l) {
	ASSERT_NEAR(0.0, dpda.col(l).sum(), 1E-14) << l;
}
const double eps = 1E-8;
const double tol = 1E-7;
for (unsigned int k = 0; k < dim; ++k) {
	for (unsigned int j = 0; j < dim - 1; ++j) {
		const double orig_a = mlr.a()[j];
		mlr.a()[j] = orig_a + eps / 2;
		double p_hi = mlr.p(t, k);
		mlr.a()[j] = orig_a - eps / 2;
		double p_lo = mlr.p(t, k);
		EXPECT_NEAR((p_hi - p_lo) / eps, dpda(k, j), std::abs(dpda(k, j)) * tol) << k << " " << j;
		mlr.a()[j] = orig_a;
	}
}
}

TEST(MultinomialLogisticRegression, EstimateSSQ) {
	const unsigned int dim = 3;
	averisera::MultinomialLogisticRegression mlr(dim);
	mlr.a()[0] = 0.1;
	mlr.a()[1] = -0.1;
	mlr.b()[0] = -0.2;
	mlr.b()[1] = 0.2;
	const std::vector<double> t = { -1, 2 };
	Eigen::MatrixXd p(dim, t.size());
	std::vector<double> grad(2 * (dim - 1));
	for (unsigned int k = 0; k < dim; ++k) {
		for (unsigned int l = 0; l < t.size(); ++l) {
			p(k, l) = mlr.p(t[l], k);
		}
	}
	std::fill(mlr.a().begin(), mlr.a().end(), 0.0);
	std::fill(mlr.b().begin(), mlr.b().end(), 0.0);
	Eigen::VectorXd ns(2);
	ns[0] = 800;
	ns[1] = 1200;
	averisera::SSQDivergence ssqdivg(p, ns);
	const double ssq = averisera::MultinomialLogisticRegression::estimate_ssq(mlr, t, p, ssqdivg.weights());
	double result_ssq = 0;
	std::vector<double> work(dim);
	for (unsigned int l = 0; l < t.size(); ++l) {
		result_ssq += mlr.ssq(t[l], p.col(l), ssqdivg.weights()[l], work);
	}
	ASSERT_NEAR(0.0, ssq, 1E-16);
	ASSERT_EQ(ssq, result_ssq);
	const double param_tol = 1E-15;
	ASSERT_NEAR(0.1, mlr.a()[0], param_tol);
	ASSERT_NEAR(-0.1, mlr.a()[1], param_tol);
	ASSERT_NEAR(-0.2, mlr.b()[0], param_tol);
	ASSERT_NEAR(0.2, mlr.b()[1], param_tol);

	const double ssq2 = averisera::MultinomialLogisticRegression::estimate(mlr, t, p, ns, averisera::MultinomialLogisticRegression::EstimationType::SSQ);
	ASSERT_EQ(ssq, ssq2);
	ASSERT_NEAR(0.1, mlr.a()[0], param_tol);
	ASSERT_NEAR(-0.1, mlr.a()[1], param_tol);
	ASSERT_NEAR(-0.2, mlr.b()[0], param_tol);
	ASSERT_NEAR(0.2, mlr.b()[1], param_tol);
}

void test_symmetry(const char* name, const Eigen::MatrixXd& m) {
	ASSERT_EQ(m.rows(), m.cols()) << name;
	const unsigned int dim = static_cast<unsigned int>(m.rows());
	for (unsigned int c = 0; c < dim; ++c) {
		for (unsigned int r = 0; r < c; ++r) {
			ASSERT_NEAR(m(r, c), m(c, r), 1E-14) << name << ": " << r << " " << c;
		}
	}
}

void test_positive_definite(const char* name, const Eigen::MatrixXd& m) {
	Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigensolver(m, Eigen::EigenvaluesOnly);
	//std::cout << name << ": " << eigensolver.eigenvalues() << std::endl;
	const unsigned int dim = static_cast<unsigned int>(m.rows());
	for (unsigned int k = 0; k < dim; ++k) {
		ASSERT_TRUE(m(k, k) >= 0) << name << ": " << k;
		ASSERT_TRUE(eigensolver.eigenvalues()[k] >= 0) << name << ": " << k << " " << eigensolver.eigenvalues()[k];
	}
}

TEST(MultinomialLogisticRegression, EstimateLogLikelihood) {
	const unsigned int dim = 3;
	averisera::MultinomialLogisticRegression mlr(dim);
	mlr.a()[0] = 0.1;
	mlr.a()[1] = -0.1;
	mlr.b()[0] = -0.2;
	mlr.b()[1] = 0.2;
	const std::vector<double> t = { -1, 2 };
	Eigen::MatrixXd p(dim, t.size());
	std::vector<double> grad(2 * (dim - 1));
	for (unsigned int k = 0; k < dim; ++k) {
		for (unsigned int l = 0; l < t.size(); ++l) {
			p(k, l) = mlr.p(t[l], k);
		}
	}
	std::fill(mlr.a().begin(), mlr.a().end(), 0.0);
	std::fill(mlr.b().begin(), mlr.b().end(), 0.0);
	Eigen::VectorXd ns(2);
	ns[0] = 800;
	ns[1] = 1200;
	const double ll = averisera::MultinomialLogisticRegression::estimate(mlr, t, p, ns, averisera::MultinomialLogisticRegression::EstimationType::MLE);
	ASSERT_NEAR(0.0, ll, 1E-14);
	const double param_tol = 1E-15;
	ASSERT_NEAR(0.1, mlr.a()[0], param_tol);
	ASSERT_NEAR(-0.1, mlr.a()[1], param_tol);
	ASSERT_NEAR(-0.2, mlr.b()[0], param_tol);
	ASSERT_NEAR(0.2, mlr.b()[1], param_tol);

	// Test inverse covariance matrix of parameters
	Eigen::MatrixXd inv_cov;
	const unsigned int cov_dim = 2 * (dim - 1);
	mlr.calc_param_inverse_covariance_matrix(t, ns, inv_cov);
	ASSERT_EQ(static_cast<unsigned int>(inv_cov.rows()), cov_dim);
	test_symmetry("inv_cov", inv_cov);
	test_positive_definite("inv_cov", inv_cov);
	// Test covariance matrix of parameters
	Eigen::MatrixXd cov;
	mlr.calc_param_covariance_matrix(t, ns, cov);
	ASSERT_EQ(static_cast<unsigned int>(cov.rows()), cov_dim);
	test_symmetry("cov", cov);
	test_positive_definite("cov", cov);
	ASSERT_NEAR(0.0, (cov * inv_cov - Eigen::MatrixXd::Identity(cov_dim, cov_dim)).norm(), 1E-14);
	ASSERT_NEAR(0.0, (inv_cov * cov - Eigen::MatrixXd::Identity(cov_dim, cov_dim)).norm(), 1E-14);

	// Test covariance matrix of w variables
	const double tau = -0.5;
	const unsigned int w_dim = dim - 1;
	Eigen::MatrixXd w_cov;
	mlr.calc_w_covariance_matrix(cov, tau, w_cov);
	ASSERT_EQ(static_cast<unsigned int>(w_cov.rows()), w_dim);
	test_symmetry("w_cov", w_cov);
	test_positive_definite("w_cov", w_cov);
	
	// Test confidence intervals for extrapolated probabilities
	Eigen::VectorXd probs(dim);
	mlr.p(tau, probs);
	Eigen::VectorXd lower(dim);
	Eigen::VectorXd upper(dim);
	mlr.calc_confidence_intervals(w_cov, probs, 0.95, lower, upper);
	for (unsigned int i = 0; i < dim; ++i) {
		ASSERT_TRUE(lower[i] < probs[i]) << i;
		ASSERT_TRUE(upper[i] > probs[i]) << i;
	}
}

TEST(MultinomialLogisticRegression, InitGuess) {
	const unsigned int dim = 2;
	averisera::MultinomialLogisticRegression mlr(dim);
	const std::vector<double> t = { -1, 2, 4 };
	Eigen::MatrixXd pTrans(t.size(), dim);
	pTrans << 0.331812228, 0.668187772,
		0.475020813, 0.524979187,
		0.574442517, 0.425557483;
	std::vector<double> x(2);
	mlr.init_guess(t, pTrans.transpose(), x);
	ASSERT_NEAR(0.5, x[0], 2E-9);
	ASSERT_NEAR(-0.2, x[1], 1E-10);
}

TEST(MultinomialLogisticRegression, SteadyState) {
	averisera::MultinomialLogisticRegression mlr(3);
	Eigen::VectorXd ss(3);

	mlr.a()[0] = 0.1;
	mlr.a()[1] = -0.1;
	mlr.b()[0] = -0.2;
	mlr.b()[1] = 0.2;	
	mlr.calc_steady_state(ss);
	ASSERT_EQ(0, ss[0]);
	ASSERT_EQ(0, ss[1]);
	ASSERT_EQ(1, ss[2]);

	mlr.a()[0] = 0.1;
	mlr.a()[1] = -0.1;
	mlr.b()[0] = -0.2;
	mlr.b()[1] = -0.25;
	mlr.calc_steady_state(ss);
	ASSERT_EQ(1, ss[0]);
	ASSERT_EQ(0, ss[1]);
	ASSERT_EQ(0, ss[2]);

	mlr.a()[0] = 0.1;
	mlr.a()[1] = -0.1;
	mlr.b()[0] = 0.2;
	mlr.b()[1] = -0.2;
	mlr.calc_steady_state(ss);
	ASSERT_EQ(0, ss[0]);
	ASSERT_EQ(1, ss[1]);
	ASSERT_EQ(0, ss[2]);

	mlr.a()[0] = 0.1;
	mlr.a()[1] = -0.1;
	mlr.b()[0] = 0.2;
	mlr.b()[1] = 0.25;
	mlr.calc_steady_state(ss);
	ASSERT_EQ(0, ss[0]);
	ASSERT_EQ(0, ss[1]);
	ASSERT_EQ(1, ss[2]);

	mlr.a()[0] = 0.1;
	mlr.a()[1] = 0.1;
	mlr.b()[0] = 0.2;
	mlr.b()[1] = 0.2;
	mlr.calc_steady_state(ss);
	ASSERT_EQ(0, ss[0]);
	ASSERT_NEAR(0.5, ss[1], 1E-15);
	ASSERT_NEAR(0.5, ss[2], 1E-15);
}

TEST(MultinomialLogisticRegression, ModelMLE) {
	const unsigned int dim = 2;
	averisera::MultinomialLogisticRegression::Model model(dim);
	const unsigned int T = 4;
	averisera::ObservedDiscreteData data(dim, T);
	data.nbr_surveys.setConstant(100.);
	std::iota(data.times.begin(), data.times.end(), 0.);
	data.probs << 0.1, 0.11, 0.12, 0.13,
		0.9, 0.89, 0.88, 9.87;
	std::vector<double> extrap_times(data.times);
	Eigen::MatrixXd extrap_probs(dim, T);
	double norm = model(data, extrap_times, extrap_probs);
	double norm2 = 0.;
	for (unsigned int k = 0; k < T; ++k) {
		norm2 += averisera::Statistics::weighted_kl_divergence(data.nbr_surveys[k], data.probs.col(k), extrap_probs.col(k));
	}
	ASSERT_NEAR(norm2, norm, 1E-15);
	extrap_times[1] = -10;
	norm = model(data, extrap_times, extrap_probs);
	for (unsigned int k = 0; k < T; ++k) {
		norm2 += averisera::Statistics::weighted_kl_divergence(data.nbr_surveys[k], data.probs.col(k), extrap_probs.col(k));
	}
	ASSERT_GT(norm2, norm) << norm2;
}

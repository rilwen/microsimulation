// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include <iostream>
#include <random>
#include "core/csm.hpp"
#include "core/csm_objective_function.hpp"
#include "core/csm_regulariser_nearest_neighbours.hpp"
#include "core/csm_utils.hpp"
#include "core/bootstrap.hpp"
#include "core/statistics.hpp"
#include "core/crossvalidation.hpp"
#include "core/observed_discrete_data.hpp"

using namespace averisera;

TEST(CSM, CalcArgDim) {
	ASSERT_EQ(6u, CSM::calc_arg_dim(2, 0));
	ASSERT_EQ(12u, CSM::calc_arg_dim(2, 1));
	ASSERT_EQ(24u, CSM::calc_arg_dim(2, 2));
}

static void test_stylised(const double error_tol, const double pi_tol, const double q0_tol) {
	const unsigned int dim = 3;
	Eigen::MatrixXd expected_pi(dim, dim);
	expected_pi.col(0) = Eigen::Vector3d(0.8, 0.1, 0.1);
	expected_pi.col(1) = Eigen::Vector3d(0.05, 0.9, 0.05);
	expected_pi.col(2) = Eigen::Vector3d(0.05, 0.15, 0.8);

	const unsigned int T = 10;
	ObservedDiscreteData data(dim, T);
	Eigen::MatrixXd& p = data.probs;
	p.col(0) = Eigen::Vector3d(0.26, 0.41, 0.33);
	/*p.col(1) = Eigen::Vector3d(0.245, 0.4445, 0.3105);
	p.col(2) = Eigen::Vector3d(0.23375, 0.471125, 0.295125);
	p.col(3) = Eigen::Vector3d(0.225313, 0.491656, 0.283031);*/
	for (unsigned int t = 1; t < T; ++t) {
		p.col(t) = expected_pi * p.col(t - 1);
	}

	// Optimized function - for comparison
	const unsigned int memory = 0;
	CSMObjectiveFunction f(data, CSMParams(memory, 1., 0, 0, nullptr));
	std::vector<double> x(f.arg_dim());
	std::vector<double> grad(f.arg_dim());
	// p.col(0) is converted into a temporary vector object
	Eigen::VectorXd q0(p.col(0));
	CSMUtils::copy_probabilities(Eigen::MatrixXd::Identity(dim, dim), dim, q0, x, memory);
	const double initial_y = f.value(x, grad, true);

	CSM est(data, CSMParams(memory, 1, 0, 0, nullptr));
	ASSERT_EQ(dim, est.dim());
	Eigen::MatrixXd pi;
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::HEURISTIC);
	const double y = est.estimate(pi, q0);
	// Check dimensions
	ASSERT_EQ(dim, static_cast<unsigned int>(pi.rows()));
	ASSERT_EQ(dim, static_cast<unsigned int>(pi.cols()));
	ASSERT_EQ(dim, static_cast<unsigned int>(q0.size()));

	// Check if we obtained a reduction in error
	EXPECT_TRUE(y < initial_y) << "Initial Y: " << initial_y << ", optimised Y: " << y << std::endl;

	// Check if the error is small
	EXPECT_NEAR(0.0, y, error_tol);
	//std::cout << "y == " << y << std::endl;

	// Check if we recovered the true pi
	const double x_diff = (expected_pi - pi).norm();
	// Test and print a detailed error message if test fails.
	// EXPECT_NEAR does not stop the test on failure, as opposed to ASSERT_NEAR
	// We use a large tolerance because fitting the transition matrix to probability distributions is not easy.
	EXPECT_NEAR(0.0, x_diff, pi_tol) << "Expected pi_expanded: " << expected_pi << std::endl << "Actual pi_expanded: " << pi << std::endl;

	// Check if we recovered the true initial state
	EXPECT_NEAR(0.0, (q0 - p.col(0)).norm(), q0_tol) << "Expected initial state: " << p.col(0).transpose() << std::endl << "Actual initial state: " << q0 << std::endl;

	// check constraints
	for (unsigned int k = 0; k < dim; ++k) {
		EXPECT_NEAR(1.0, pi.col(k).sum(), 1E-14);
		for (unsigned int l = 0; l < dim; ++l) {
			EXPECT_TRUE(pi(k, l) >= 0.0);
			EXPECT_TRUE(pi(k, l) <= 1.0);
		}
	}

	CSMUtils::copy_probabilities(pi, dim, q0, x, memory);
	EXPECT_NEAR(f.value(x, grad, true), y, 1E-12) << "Mismatch between estimate() and direct call to optimized function";
}

TEST(CSM,StylisedInitState) {
	test_stylised(2E-14, 1E-4, 1E-7);
}

TEST(CSM, Dof) {
	const unsigned int dim = 3;
	const unsigned int memory = 1;
	const unsigned int T = 10;
	ObservedDiscreteData data(dim, T);
	CSM est(data, CSMParams(memory, 1, 0, 0, nullptr));
	ASSERT_EQ(dim*dim*(dim - 1) + dim*dim - 1, est.dof());
}

TEST(CSM, PaddedInputs) {
	const unsigned int dim = 3;
	Eigen::MatrixXd expected_pi(dim, dim);
	expected_pi.col(0) = Eigen::Vector3d(0.8, 0.1, 0.1);
	expected_pi.col(1) = Eigen::Vector3d(0.05, 0.9, 0.05);
	expected_pi.col(2) = Eigen::Vector3d(0.05, 0.15, 0.8);

	const unsigned int T = 10;
	ObservedDiscreteData data(dim, T);
	Eigen::MatrixXd& p = data.probs;
	const unsigned int year_step = 2;
	p.col(0) = Eigen::Vector3d(0.26, 0.41, 0.33);
	Eigen::VectorXd tmp(dim);
	const double start_year = 2000;
	data.times[0] = start_year;
	for (unsigned int t = 1; t < T; ++t) {
		// Avoid aliasing
		tmp = p.col(t - 1);
		for (unsigned int s = 0; s < year_step; ++s) {
			p.col(t) = expected_pi * tmp;
			tmp = p.col(t);
		}

		// set year
		data.times[t] = start_year + t * year_step;
	}

	CSM est(data, CSMParams(0, 1, 0, 0, nullptr));
	Eigen::MatrixXd pi;
	Eigen::VectorXd q0;
	est.calc_initial_guess_q0(q0, CSM::InitialStateDistributionInitialisationMethod::FROM_DATA);
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::HEURISTIC);
	std::string est_info;
	const double y = est.estimate(pi, q0, &est_info);
	ASSERT_FALSE(est_info.empty());
	// Check dimensions
	ASSERT_EQ(dim, static_cast<unsigned int>(pi.rows()));
	ASSERT_EQ(dim, static_cast<unsigned int>(pi.cols()));
	ASSERT_EQ(dim, static_cast<unsigned int>(q0.size()));

	Eigen::MatrixXd errors;
	est.calc_errors(pi, q0, errors);
	EXPECT_EQ(T, static_cast<unsigned int>(errors.cols())) << "Expected errors only for input years";
	for (unsigned int t = 0; t < T; ++t) {
		EXPECT_TRUE(errors.col(t).norm() > 0) << "Expected nonzero errors for every year: " << t;
	}
	Eigen::MatrixXd bootstrapped_errors(dim, T);
	const unsigned int extrapT = year_step * T + 10;
	Eigen::MatrixXd extrap_probs(dim, extrapT);
	CSMUtils::extrapolate(pi, q0, extrap_probs);

	const double error_tol = 2E-13;
	const double pi_tol = 1E-4;

	// Check if the error is small
	EXPECT_NEAR(0.0, y, error_tol);
	//std::cout << "y == " << y << std::endl;

	// Check if we recovered the true pi
	const double x_diff = (expected_pi - pi).norm();
	// Test and print a detailed error message if test fails.
	// EXPECT_NEAR does not stop the test on failure, as opposed to ASSERT_NEAR
	// We use a large tolerance because fitting the transition matrix to probability distributions is not easy.
	EXPECT_NEAR(0.0, x_diff, pi_tol) << "Expected pi_expanded: " << expected_pi << std::endl << "Actual pi_expanded: " << pi << std::endl;

	const double q0_tol = 1E-6;
	// Check if we recovered the true initial state
	EXPECT_NEAR(0.0, (q0 - p.col(0)).norm(), q0_tol) << "Expected initial state: " << p.col(0).transpose() << std::endl << "Actual initial state: " << q0 << std::endl;

	// Compare with calculation without padding
	Eigen::MatrixXd pi_no_padding;
	Eigen::VectorXd q0_no_padding;	
	ObservedDiscreteData data_no_padding(data);
	std::iota(data_no_padding.times.begin(), data_no_padding.times.end(), 0);
	CSM est_no_padding(data_no_padding, CSMParams(0, 1, 0, 0, nullptr));
	est_no_padding.calc_initial_guess_pi(pi_no_padding, CSM::TransitionMatrixInitialisationMethod::HEURISTIC);
	est_no_padding.calc_initial_guess_q0(q0_no_padding, CSM::InitialStateDistributionInitialisationMethod::FROM_DATA);
	const double y_no_padding = est_no_padding.estimate(pi_no_padding, q0_no_padding);
	EXPECT_NEAR(0.0, y_no_padding, error_tol);
	EXPECT_NEAR(0.0, (q0 - q0_no_padding).norm(), q0_tol);
	Eigen::MatrixXd tmp_pi = pi;
	Eigen::MatrixXd expected_pi_no_padding;
	for (unsigned int s = 1; s < year_step; ++s) {
		expected_pi_no_padding = pi * tmp_pi;
		tmp_pi = expected_pi_no_padding;
	}
	EXPECT_NEAR(0.0, (expected_pi_no_padding - pi_no_padding).norm(), pi_tol);

	Eigen::MatrixXd errors_no_padding;
	est_no_padding.calc_errors(pi_no_padding, q0_no_padding, errors_no_padding);
	EXPECT_NEAR(0.0, (errors - errors_no_padding).norm(), 1E-6) << "Expected errors: " << errors << "\n" << "Actual errors: " << errors_no_padding << "\n";
	Eigen::MatrixXd extrap_probs_no_padding(dim, extrapT);
	CSMUtils::extrapolate(pi_no_padding, q0_no_padding, extrap_probs_no_padding);
}

TEST(CSM, ExtrapolateForYears) {
	const unsigned int dim = 3;
	Eigen::MatrixXd pi(dim, dim);
	pi.col(0) = Eigen::Vector3d(0.8, 0.1, 0.1);
	pi.col(1) = Eigen::Vector3d(0.05, 0.9, 0.05);
	pi.col(2) = Eigen::Vector3d(0.25, 0.15, 0.6);
	Eigen::VectorXd q0 = Eigen::Vector3d(0.2, 0.3, 0.5);
	// extrapolate for 50 years from now
	const unsigned int T = 50;
	Eigen::MatrixXd ep1(dim, T);
	CSMUtils::extrapolate(pi, q0, ep1);
	const double year0 = 2000;
	const std::vector<double> ey = { 2000, 2002, 2010 };
	Eigen::MatrixXd ep2(dim, ey.size());
	CSMUtils::extrapolate(pi, q0, year0, ey, ep2);
	ASSERT_EQ(dim, static_cast<unsigned int>(ep2.rows()));
	ASSERT_EQ(ey.size(), static_cast<size_t>(ep2.cols()));
	for (unsigned int i = 0; i < ey.size(); ++i) {
		ASSERT_NEAR(0.0, (ep1.col(static_cast<int>(ey[i] - year0)) - ep2.col(i)).norm(), 1E-15) << i;
	}
	Eigen::MatrixXd ep3(dim, T);
	Eigen::MatrixXd ep3l(dim, T);
	Eigen::MatrixXd ep3u(dim, T);
	ObservedDiscreteData data(dim, 10);
	data.probs = ep1.block(0, 0, dim, 10);
	CSM modelA(data, CSMParams(0, 1.0, dim, 0, nullptr));
	modelA.extrapolate_analytic_confidence_intervals(pi, q0, ep3, ep3l, ep3u, 0.95);
	ASSERT_NEAR(0.0, (ep3 - ep1).norm(), 1E-12);	
	for (unsigned int t = 0; t < T; ++t) {
		for (unsigned int k = 0; k < dim; ++k) {
			ASSERT_LE(0, ep3l(k, t)) << t << " " << k;
			ASSERT_LE(ep3l(k, t), ep3(k, t)) << t << " " << k;
			ASSERT_GE(ep3u(k, t), ep3(k, t)) << t << " " << k;
			ASSERT_GE(1, ep3u(k, t)) << t << " " << k;
		}
	}
}

TEST(CSM, Model) {
	const unsigned int dim = 3;
	Eigen::MatrixXd expected_pi(dim, dim);
	expected_pi.col(0) = Eigen::Vector3d(0.8, 0.1, 0.1);
	expected_pi.col(1) = Eigen::Vector3d(0.05, 0.9, 0.05);
	expected_pi.col(2) = Eigen::Vector3d(0.05, 0.15, 0.8);

	const unsigned int T = 10;
	ObservedDiscreteData data(dim, T);
	Eigen::MatrixXd& p = data.probs;
	const unsigned int year_step = 2;
	p.col(0) = Eigen::Vector3d(0.26, 0.41, 0.33);
	Eigen::VectorXd tmp(dim);
	const double start_year = 2000;
	data.times[0] = start_year;
	for (unsigned int t = 1; t < T; ++t) {
		// Avoid aliasing
		tmp = p.col(t - 1);
		for (unsigned int s = 0; s < year_step; ++s) {
			p.col(t) = expected_pi * tmp;
			tmp = p.col(t);
		}

		// set year
		data.times[t] = start_year + t * year_step;
	}

	const double tr_prob_nn = 0.1;
	CSM est(data, CSMParams(0, tr_prob_nn, 0, 0, nullptr));
	Eigen::MatrixXd pi;
	Eigen::VectorXd q0;
	const std::vector<double> extrap_years = { 2001, 2003, 2004, 2002 };

	CSM::Model modelA(CSMParams(0, tr_prob_nn, dim, 0, nullptr), true, CSM::TransitionMatrixInitialisationMethod::HEURISTIC,
		CSM::InitialStateDistributionInitialisationMethod::FROM_DATA,
		CSM::get_default_stopping_conditions(), CSM::default_algorithm);
	Eigen::MatrixXd epA(dim, extrap_years.size());
	modelA(data, extrap_years, epA);	
	
	est.calc_initial_guess_q0(q0, CSM::InitialStateDistributionInitialisationMethod::FROM_DATA);
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::IDENTITY);
	est.estimate(pi, q0);
	Eigen::MatrixXd ep0(dim, extrap_years.size());
	CSMUtils::extrapolate(pi, q0, static_cast<int>(start_year), extrap_years, ep0);

	CSM::Model modelB(CSMParams(0, tr_prob_nn, dim, 0, nullptr), false, CSM::TransitionMatrixInitialisationMethod::HEURISTIC,
		CSM::InitialStateDistributionInitialisationMethod::FROM_DATA,
		CSM::get_default_stopping_conditions(), CSM::default_algorithm);
	Eigen::MatrixXd epB(dim, extrap_years.size());
	modelB(data, extrap_years, epB);

	EXPECT_EQ(0.0, (epA - epB).norm());
	EXPECT_NEAR(0.0, (ep0 - epB).norm(), 1E-15);
	EXPECT_NEAR(0.0, (ep0 - epA).norm(), 1E-15);
}

TEST(CSM, LOOCV) {
	const unsigned int dim = 3;
	Eigen::MatrixXd expected_pi(dim, dim);
	expected_pi.col(0) = Eigen::Vector3d(0.8, 0.1, 0.1);
	expected_pi.col(1) = Eigen::Vector3d(0.05, 0.9, 0.05);
	expected_pi.col(2) = Eigen::Vector3d(0.05, 0.15, 0.8);

	const unsigned int T = 10;
	ObservedDiscreteData data(dim, T);
	data.nbr_surveys[0] = 10000;
	data.nbr_surveys[1] = 12000;
	data.nbr_surveys[2] = 8000;
	data.nbr_surveys[3] = 14000;
	data.nbr_surveys[4] = 10000;
	data.nbr_surveys[5] = 6000;
	data.nbr_surveys[6] = 12000;
	data.nbr_surveys[7] = 11000;
	data.nbr_surveys[8] = 10000;
	data.nbr_surveys[9] = 15000;
	Eigen::MatrixXd& p = data.probs;
	const unsigned int year_step = 1;
	p.col(0) = Eigen::Vector3d(0.26, 0.41, 0.33);
	Eigen::VectorXd tmp(dim);
	std::vector<double>& years = data.times;
	const double start_year = 2000;
	years[0] = start_year;
	for (unsigned int t = 1; t < T; ++t) {
		// Avoid aliasing
		tmp = p.col(t - 1);
		for (unsigned int s = 0; s < year_step; ++s) {
			p.col(t) = expected_pi * tmp;
			tmp = p.col(t);
		}

		// set year
		years[t] = start_year + t * year_step;
	}

	const double tr_prob_nn = 1;
	CSM::Model model(CSMParams(0, tr_prob_nn, dim, 0, nullptr), true, CSM::TransitionMatrixInitialisationMethod::HEURISTIC,
		CSM::InitialStateDistributionInitialisationMethod::FROM_DATA,
		CSM::get_default_stopping_conditions(), CSM::default_algorithm);
	auto divergence_kl = [](double ns, Eigen::Ref<const Eigen::VectorXd> P, Eigen::MatrixXd::ColXpr Q) { return Statistics::weighted_kl_divergence(ns, P, Q); };
	const std::vector<double> result = CrossValidation::cross_validation(data, model, divergence_kl, false);
	ASSERT_EQ(T, result.size());
	for (unsigned int t = 0; t < T; ++t) {
		const double tol = t == 0 ? 5E-6 : 5E-11; // for t == 0 we extrapolate using inverse of pi, which causes higher error
		EXPECT_NEAR(0.0, result[t], tol) << t;
	}
}

TEST(CSM, kFold) {
	const unsigned int dim = 3;
	Eigen::MatrixXd expected_pi(dim, dim);
	expected_pi.col(0) = Eigen::Vector3d(0.8, 0.1, 0.1);
	expected_pi.col(1) = Eigen::Vector3d(0.05, 0.9, 0.05);
	expected_pi.col(2) = Eigen::Vector3d(0.05, 0.15, 0.8);

	const unsigned int T = 10;
	ObservedDiscreteData data(dim, T);
	data.nbr_surveys[0] = 10000;
	data.nbr_surveys[1] = 12000;
	data.nbr_surveys[2] = 8000;
	data.nbr_surveys[3] = 14000;
	data.nbr_surveys[4] = 10000;
	data.nbr_surveys[5] = 6000;
	data.nbr_surveys[6] = 12000;
	data.nbr_surveys[7] = 11000;
	data.nbr_surveys[8] = 10000;
	data.nbr_surveys[9] = 15000;
	Eigen::MatrixXd& p = data.probs;
	const unsigned int year_step = 1;
	p.col(0) = Eigen::Vector3d(0.26, 0.41, 0.33);
	Eigen::VectorXd tmp(dim);
	std::vector<double>& years = data.times;
	const double start_year = 2000;
	years[0] = start_year;
	for (unsigned int t = 1; t < T; ++t) {
		// Avoid aliasing
		tmp = p.col(t - 1);
		for (unsigned int s = 0; s < year_step; ++s) {
			p.col(t) = expected_pi * tmp;
			tmp = p.col(t);
		}

		// set year
		years[t] = start_year + t * year_step;
	}
	std::mt19937 urng;
	urng.seed(42);
	const double tr_prob_nn = 1;
	CSM::Model model(CSMParams(0, tr_prob_nn, dim, 0, nullptr), true, CSM::TransitionMatrixInitialisationMethod::HEURISTIC,
		CSM::InitialStateDistributionInitialisationMethod::FROM_DATA,
		CSM::get_default_stopping_conditions(), CSM::default_algorithm);
	auto divergence_kl = [](double ns, Eigen::MatrixXd::ConstColXpr P, Eigen::MatrixXd::ColXpr Q) { return Statistics::weighted_kl_divergence(ns, P, Q); };
	const unsigned int kfold_k = 3;
	const auto result = CrossValidation::cross_validation_kfold(data, kfold_k, model, divergence_kl, urng, false);
	ASSERT_EQ(T, result.second.size());
	ASSERT_EQ(kfold_k, result.first.size());
	for (unsigned int t = 0; t < kfold_k; ++t) {
		EXPECT_NEAR(0.0, result.first[t], 1E-10) << t;
	}
}

TEST(CSM, CalcInitialGuessPiNoMemoryHeuristic) {
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim, 2);

	CSM est(data, CSMParams(0, 1, dim, 0, nullptr));

	Eigen::MatrixXd pi;
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::HEURISTIC);
	ASSERT_EQ(dim, pi.rows());
	ASSERT_EQ(dim, pi.cols());
	ASSERT_NEAR(0.0, (pi - Eigen::MatrixXd::Identity(dim, dim)).norm(), 1E-15);
}

TEST(CSM, CalcInitialGuessPiNoMemoryIdentity) {
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim, 2);

	CSM est(data, CSMParams(0, 1, dim, 0, nullptr));

	Eigen::MatrixXd pi;
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::IDENTITY);
	ASSERT_EQ(dim, pi.rows());
	ASSERT_EQ(dim, pi.cols());
	ASSERT_NEAR(0.0, (pi - Eigen::MatrixXd::Identity(dim, dim)).norm(), 1E-15);
}

TEST(CSM, CalcInitialGuessPiNoMemoryMaxEntropy) {
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim, 2);

	CSM est(data, CSMParams(0, 1, dim, 0, nullptr));

	Eigen::MatrixXd pi;
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::MAX_ENTROPY);
	ASSERT_EQ(dim, pi.rows());
	ASSERT_EQ(dim, pi.cols());
	ASSERT_NEAR(0.0, (pi - Eigen::MatrixXd::Constant(dim, dim, 1.0 / dim)).norm(), 1E-15);
}

TEST(CSM, CalcInitialGuessPiNoMemoryFromTrajectoriesNoLongitudinalData) {
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim, 2);

	CSM est(data, CSMParams(0, 1, dim, 0, nullptr));

	Eigen::MatrixXd pi;
	ASSERT_THROW(est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::FROM_TRAJECTORIES), std::logic_error);
}

TEST(CSM, CalcInitialGuessPiNoMemoryFromTrajectoriesCompleteOnlyNoLongitudinalData) {
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim, 2);

	CSM est(data, CSMParams(0, 1, dim, 0, nullptr));

	Eigen::MatrixXd pi;
	ASSERT_THROW(est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::FROM_TRAJECTORIES_COMPLETE_ONLY), std::logic_error);
}

TEST(CSM, CalcInitialGuessPiNoMemoryFromTrajectoriesWithLongitudinalData) {
	const unsigned int dim = 2;

	ObservedDiscreteData data(dim);
	
	data.ltrajs.init_rectangular(3, 2);
	data.ltimes.init_rectangular(3, 2);

	data.ltrajs(0, 0) = 0;
	data.ltrajs(0, 1) = 1;
	data.ltrajs(1, 0) = 0;
	data.ltrajs(1, 1) = 1;
	data.ltrajs(2, 0) = 1;
	data.ltrajs(2, 1) = 1;	

	data.ltimes(0, 0) = 0;
	data.ltimes(0, 1) = 1;
	data.ltimes(1, 0) = 0;
	data.ltimes(1, 1) = 1;
	data.ltimes(2, 0) = 0;
	data.ltimes(2, 1) = 2;

	CSM est(data, CSMParams(0, 1, dim, 0, nullptr));

	Eigen::MatrixXd expected(dim, dim);
	expected(0, 0) = 0;
	expected(1, 0) = 1;
	expected(0, 1) = 0.5;
	expected(1, 1) = 0.5;

	Eigen::MatrixXd pi;
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::FROM_TRAJECTORIES);
	ASSERT_EQ(expected.rows(), pi.rows());
	ASSERT_EQ(expected.cols(), pi.cols());
	ASSERT_TRUE(pi.isApprox(expected)) << pi;
}

TEST(CSM, CalcInitialGuessPiNoMemoryFromTrajectoriesCompleteOnlyWithLongitudinalData) {
	const unsigned int dim = 2;

	ObservedDiscreteData data(dim);

	data.ltrajs.init_rectangular(3, 2);
	data.ltimes.init_rectangular(3, 2);

	data.ltrajs(0, 0) = 0;
	data.ltrajs(0, 1) = 1;
	data.ltrajs(1, 0) = 0;
	data.ltrajs(1, 1) = 1;
	data.ltrajs(2, 0) = 1;
	data.ltrajs(2, 1) = 1;

	data.ltimes(0, 0) = 0;
	data.ltimes(0, 1) = 1;
	data.ltimes(1, 0) = 0;
	data.ltimes(1, 1) = 1;
	data.ltimes(2, 0) = 1;
	data.ltimes(2, 1) = 3;

	CSM est(data, CSMParams(0, 1, dim, 0, nullptr));

	Eigen::MatrixXd expected(dim, dim);
	expected(0, 0) = 0;
	expected(1, 0) = 1;
	expected(0, 1) = 0.5;
	expected(1, 1) = 0.5;

	Eigen::MatrixXd pi;
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::FROM_TRAJECTORIES_COMPLETE_ONLY);
	ASSERT_EQ(expected.rows(), pi.rows());
	ASSERT_EQ(expected.cols(), pi.cols());
	ASSERT_TRUE(pi.isApprox(expected)) << pi;
}

TEST(CSM, CalcInitialGuessPiMemoryHeuristic) {
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim, 2);
	Eigen::MatrixXd& p = data.probs;
	p.setConstant(1.0 / dim);

	CSM est(data, CSMParams(2, 1, 0, 0, nullptr));

	Eigen::MatrixXd pi;	
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::HEURISTIC);	
	
	Eigen::MatrixXd expected = Eigen::MatrixXd::Zero(est.dim(), est.state_dim());
	// States are arranged thus: (0,0,0), (1,0,0), (0,1,0), (1,1,0), (0,0,1), (1,0,1), (0,1,1), (1,1,1)
	expected(0, 0) = 1.0;
	expected(1, 1) = 1.0;
	expected(0, 2) = 1.0;
	expected(1, 3) = 1.0;
	expected(0, 4) = 1.0;
	expected(1, 5) = 1.0;
	expected(0, 6) = 1.0;
	expected(1, 7) = 1.0;
	ASSERT_EQ(expected.rows(), pi.rows());
	ASSERT_EQ(expected.cols(), pi.cols());
	ASSERT_TRUE(pi.isApprox(expected, 1E-13));
}

TEST(CSM, CalcInitialGuessPiMemoryIdentity) {
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim, 2);
	Eigen::MatrixXd& p = data.probs;
	p.setConstant(1.0 / dim);

	CSM est(data, CSMParams(2, 1, 0, 0, nullptr));

	Eigen::MatrixXd pi;
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::HEURISTIC);

	Eigen::MatrixXd expected = Eigen::MatrixXd::Zero(est.dim(), est.state_dim());
	// States are arranged thus: (0,0,0), (1,0,0), (0,1,0), (1,1,0), (0,0,1), (1,0,1), (0,1,1), (1,1,1)
	expected(0, 0) = 1.0;
	expected(1, 1) = 1.0;
	expected(0, 2) = 1.0;
	expected(1, 3) = 1.0;
	expected(0, 4) = 1.0;
	expected(1, 5) = 1.0;
	expected(0, 6) = 1.0;
	expected(1, 7) = 1.0;
	ASSERT_EQ(expected.rows(), pi.rows());
	ASSERT_EQ(expected.cols(), pi.cols());
	ASSERT_TRUE(pi.isApprox(expected, 1e-13));
}

TEST(CSM, CalcInitialGuessPiMemoryMaxEntropy) {
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim, 2);	

	CSM est(data, CSMParams(2, 1, dim, 0, nullptr));

	Eigen::MatrixXd pi;
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::MAX_ENTROPY);
	ASSERT_NEAR(0.0, (pi - Eigen::MatrixXd::Constant(dim, est.state_dim(), 1.0 / dim)).norm(), 1E-15);
}

TEST(CSM, CalcInitialGuessPiMemoryFromTrajectoriesNoLongitudinalData) {
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim, 2);

	CSM est(data, CSMParams(2, 1, dim, 0, nullptr));

	Eigen::MatrixXd pi;
	ASSERT_THROW(est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::FROM_TRAJECTORIES), std::logic_error);
}

TEST(CSM, CalcInitialGuessPiMemoryFromTrajectoriesCompleteOnlyNoLongitudinalData) {
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim, 2);

	CSM est(data, CSMParams(2, 1, dim, 0, nullptr));

	Eigen::MatrixXd pi;
	ASSERT_THROW(est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::FROM_TRAJECTORIES_COMPLETE_ONLY), std::logic_error);
}

TEST(CSM, CalcInitialGuessPiMemoryFromTrajectoriesCompleteOnlyWithLongitudinalData) {
	const unsigned int dim = 2;

	ObservedDiscreteData data(dim);

	const unsigned int traj_len = 3;
	const unsigned int n_trajs = 7;

	data.ltrajs.init_rectangular(n_trajs, traj_len);
	data.ltimes.init_rectangular(n_trajs, traj_len);

	data.ltrajs(0, 0) = 0;
	data.ltrajs(0, 1) = 0;
	data.ltrajs(0, 2) = 0;

	data.ltrajs(1, 0) = 0;
	data.ltrajs(1, 1) = 0;
	data.ltrajs(1, 2) = 1;

	data.ltrajs(2, 0) = 1;
	data.ltrajs(2, 1) = 1;
	data.ltrajs(2, 2) = 0;

	data.ltrajs(3, 0) = 1;
	data.ltrajs(3, 1) = 1;
	data.ltrajs(3, 2) = 1;

	data.ltrajs(4, 0) = 0;
	data.ltrajs(4, 1) = 1;
	data.ltrajs(4, 2) = 0;

	data.ltrajs(5, 0) = 1;
	data.ltrajs(5, 1) = 0;
	data.ltrajs(5, 2) = 1;

	data.ltrajs(6, 0) = 1;
	data.ltrajs(6, 1) = 0;
	data.ltrajs(6, 2) = 1;

	for (unsigned int k = 0; k < n_trajs; ++k) {
		for (unsigned int t = 0; t < traj_len; ++t) {
			data.ltimes(k, t) = t + k;
		}
	}

	data.ltimes(5, 0) = -1;
	data.ltimes(6, 2) = 100;

	CSM est(data, CSMParams(1, 1, dim, 0, nullptr));

	Eigen::MatrixXd expected(dim, dim * dim);
	// (0, 0) -> either 0 or 1
	expected(0, 0) = 0.5;
	expected(1, 0) = 0.5;

	// (0, 1) -> 0
	expected(0, 1) = 1;
	expected(1, 1) = 0;

	// (1, 0) -> Not known (missing data)
	expected(0, 2) = 0.5;
	expected(1, 2) = 0.5;
	
	// (1, 1) -> either 0 or 1
	expected(0, 3) = 0.5;
	expected(1, 3) = 0.5;

	Eigen::MatrixXd pi;

	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::FROM_TRAJECTORIES_COMPLETE_ONLY);
	ASSERT_EQ(expected.rows(), pi.rows());
	ASSERT_EQ(expected.cols(), pi.cols());
	ASSERT_TRUE(pi.isApprox(expected)) << pi;	
}

TEST(CSM, CalcInitialGuessPiMemoryFromTrajectoriesWithLongitudinalData) {
	const unsigned int dim = 2;

	ObservedDiscreteData data(dim);

	const unsigned int traj_len = 3;
	const unsigned int n_trajs = 7;

	data.ltrajs.init_rectangular(n_trajs, traj_len);
	data.ltimes.init_rectangular(n_trajs, traj_len);

	data.ltrajs(0, 0) = 0;
	data.ltrajs(0, 1) = 0;
	data.ltrajs(0, 2) = 0;

	data.ltrajs(1, 0) = 0;
	data.ltrajs(1, 1) = 0;
	data.ltrajs(1, 2) = 1;

	data.ltrajs(2, 0) = 1;
	data.ltrajs(2, 1) = 1;
	data.ltrajs(2, 2) = 0;

	data.ltrajs(3, 0) = 1;
	data.ltrajs(3, 1) = 1;
	data.ltrajs(3, 2) = 1;

	data.ltrajs(4, 0) = 0;
	data.ltrajs(4, 1) = 1;
	data.ltrajs(4, 2) = 0;

	data.ltrajs(5, 0) = 1;
	data.ltrajs(5, 1) = 0;
	data.ltrajs(5, 2) = 1;

	data.ltrajs(6, 0) = 1;
	data.ltrajs(6, 1) = 0;
	data.ltrajs(6, 2) = 1;

	for (unsigned int k = 0; k < n_trajs; ++k) {
		for (unsigned int t = 0; t < traj_len; ++t) {
			data.ltimes(k, t) = t + k;
		}
	}

	data.ltimes(5, 0) = -1;
	data.ltimes(6, 2) = 100;

	CSM est(data, CSMParams(1, 1, dim, 0, nullptr));

	Eigen::MatrixXd expected(dim, dim * dim);
	// (0, 0) -> 0
	expected(0, 0) = 0.5 + 1 + 0.5;
	// (0, 0) -> 1
	expected(1, 0) = 1 + 0.5 + 0.5;
	// (0, 1) -> 0
	expected(0, 1) = 1 + 0.5;
	// (0, 1) -> 1
	expected(1, 1) = 0.5 + 0.5;
	// (1, 0) -> 0
	expected(0, 2) = 0.5 + 0.5;
	// (1, 0) -> 1
	expected(1, 2) = 0.5 + 0.5;
	// (1, 1) -> 0
	expected(0, 3) = 1 + 0.5;
	// (1, 1) -> 1
	expected(1, 3) = 0.5 + 1 + 0.5;

	for (Eigen::Index i = 0; i < expected.cols(); ++i) {
		expected.col(i) /= expected.col(i).sum();
	}

	Eigen::MatrixXd pi;

	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::FROM_TRAJECTORIES);
	ASSERT_EQ(expected.rows(), pi.rows());
	ASSERT_EQ(expected.cols(), pi.cols());
	ASSERT_TRUE(pi.isApprox(expected)) << pi;
}

TEST(CSM, CalcInitGuessQ0MaxEntropy) {
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim, 2);

	CSM est(data, CSMParams(2, 1, dim, 0, nullptr));
	Eigen::VectorXd q0;
	est.calc_initial_guess_q0(q0, CSM::InitialStateDistributionInitialisationMethod::MAX_ENTROPY);
	ASSERT_EQ(est.state_dim(), q0.size());
	ASSERT_NEAR(0.0, (q0 - Eigen::VectorXd::Constant(est.state_dim(), 1.0 / est.state_dim())).norm(), 1E-15);
}

TEST(CSM, CalcInitGuessQ0FromData) {
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim, 2);
	data.probs(0, 0) = 0.2;
	data.probs(1, 0) = 0.8;

	CSM est(data, CSMParams(2, 1, dim, 0, nullptr));
	Eigen::VectorXd q0;
	est.calc_initial_guess_q0(q0, CSM::InitialStateDistributionInitialisationMethod::FROM_DATA);	
	Eigen::VectorXd expected_q0(Eigen::VectorXd::Zero(est.state_dim()));
	expected_q0(0) = 0.2;
	expected_q0(7) = 0.8;
	ASSERT_EQ(expected_q0.size(), q0.size());
	ASSERT_NEAR(0.0, (q0 - expected_q0).norm(), 1E-13);
}

TEST(CSM, Errors) {
	const unsigned int T = 3;
	const unsigned int dim = 2;
	ObservedDiscreteData data(dim, T);
	Eigen::MatrixXd& p = data.probs;
	p.col(0) = Eigen::Vector2d(0.8, 0.2);
	p.col(1) = Eigen::Vector2d(0.25, 0.75);
	p.col(2) = Eigen::Vector2d(0.8, 0.2);

	CSM est(data, CSMParams(0, 1.0, dim, 0, nullptr));

	std::vector<double> x(dim * dim + dim);
	x[0] = 0; x[1] = 1; x[2] = 1; x[3] = 0;
	for (unsigned int k = 0; k < dim; ++k) {
	    x[dim*dim + k] = p.col(0)[k]; // copy the initial state
	}
	std::vector<double> grad(dim * dim + dim);

	Eigen::MatrixXd pi(dim, dim);
	std::copy(x.begin(), x.begin() + pi.size(), pi.data());
	// Test error calculation
	Eigen::MatrixXd errors;
	est.calc_errors(pi, p.col(0), errors); // passing p.col(0) to function expecting Eigen::VectorXd is safe, becasue a vector object will be created on the fly by calling the appropriate constructor (it's just slightly inefficient but this is a test code)
	ASSERT_EQ(dim, static_cast<unsigned int>(errors.rows()));
	ASSERT_EQ(T, static_cast<unsigned int>(errors.cols()));
	EXPECT_NEAR(0, errors(0, 0), 0);
	EXPECT_NEAR(0, errors(1, 0), 0);
	EXPECT_NEAR(0.05, errors(0, 1), 1E-16);
	EXPECT_NEAR(-0.05, errors(1, 1), 1E-16);
	EXPECT_NEAR(0, errors(0, 2), 0);
	EXPECT_NEAR(0, errors(1, 2), 0);

	// Test extrapolation
	const unsigned int new_T = T + 2;
	Eigen::MatrixXd extrap(dim, new_T);
	CSMUtils::extrapolate(pi, p.col(0), extrap);
	for (unsigned int t = 0; t < new_T; ++t) {
		// pi matrix is [0 1;1 0], so extrapolated columns with even indices will be equal to p.col(0)
		if (t % 2 == 0) {
			ASSERT_NEAR(0, (extrap.col(t) - p.col(0)).norm(), 1E-15);
		}
		else {
			// if they're even, the coefficients will be swapped
			ASSERT_NEAR(p(1, 0), extrap(0, t), 1E-15);
			ASSERT_NEAR(p(0, 0), extrap(1, t), 1E-15) << t;
		}
	}
}

TEST(CSM, Longitudinal) {
	ObservedDiscreteData data(2);
	data.ltrajs.init_rectangular(6, 2);
	data.ltimes.init_rectangular(6, 2);

	data.ltrajs(0, 0) = 0;
	data.ltrajs(0, 1) = 1;
	data.ltrajs(1, 0) = 1;
	data.ltrajs(1, 1) = 0;
	data.ltrajs(2, 0) = 0;
	data.ltrajs(2, 1) = 0;
	data.ltrajs(3, 0) = 1;
	data.ltrajs(3, 1) = 1;
	data.ltrajs(4, 0) = 0;
	data.ltrajs(4, 1) = 0;
	data.ltrajs(5, 0) = 1;
	data.ltrajs(5, 1) = 1;

	data.ltimes(0, 0) = 0;
	data.ltimes(0, 1) = 1;
	data.ltimes(1, 0) = 0;
	data.ltimes(1, 1) = 1;
	data.ltimes(2, 0) = 1;
	data.ltimes(2, 1) = 2;
	data.ltimes(3, 0) = 1;
	data.ltimes(3, 1) = 2;
	data.ltimes(4, 0) = 0;
	data.ltimes(4, 1) = 2;
	data.ltimes(5, 0) = 0;
	data.ltimes(5, 1) = 3;

	CSM est(data, CSMParams(0, 1, 0, 0, nullptr));
	ASSERT_EQ(2u, est.dim());

	Eigen::MatrixXd pi;
	Eigen::VectorXd q0;
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::HEURISTIC);
	est.calc_initial_guess_q0(q0, CSM::InitialStateDistributionInitialisationMethod::FROM_DATA);
	const double error = est.estimate(pi, q0);
	ASSERT_TRUE(error < 10.);
	const double tol = 1E-8;
	for (size_t k = 0; k < 2; ++k) {
		ASSERT_NEAR(0.5, q0[k], tol) << k;
		for (size_t l = 0; l < 2; ++l) {
			ASSERT_NEAR(0.5, pi(k, l), tol) << k << " " << l;
		}
	}
}

TEST(CSM, LongitudinalMemoryLongTraj) {
	const size_t dim = 2;
	ObservedDiscreteData data(dim);
	const size_t T = 10;
	const size_t ntraj = 4;
	data.ltrajs.init_rectangular(ntraj, T);
	data.ltimes.init_rectangular(ntraj, T);

	for (size_t q = 0; q < ntraj; ++q) {
		for (size_t t = 0; t < 2*T; ++t) {
			if (t < T / 2) {
				data.ltimes(q, t) = static_cast<double>(t);
			} else if (t >= T + T / 2) {
				data.ltimes(q, t - T) = static_cast<double>(t);
			}
		}
	}
	data.ltrajs(0, 0) = 0;
	data.ltrajs(0, 1) = 0;
	data.ltrajs(1, 0) = 0;
	data.ltrajs(1, 1) = 1;
	data.ltrajs(2, 0) = 1;
	data.ltrajs(2, 1) = 0;
	data.ltrajs(3, 0) = 1;
	data.ltrajs(3, 1) = 1;

	for (size_t q = 0; q < ntraj; ++q) {
		auto prev_prev = data.ltrajs(q, 0);
		auto prev = data.ltrajs(q, 1);
		for (size_t t = 2; t < 2 * T; ++t) {
			const auto next = prev == prev_prev ? 1 - prev : prev;
			if (t < T / 2) {
				data.ltrajs(q, t) = next;
			} else if (t >= T + T / 2) {
				data.ltrajs(q, t - T) = next;
			}
			prev_prev = prev;
			prev = next;
		}
	}

	CSM est(data, CSMParams(1, 1, 0, 0, nullptr));
	ASSERT_EQ(dim, est.dim());

	Eigen::MatrixXd pi;
	Eigen::VectorXd q0;
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::HEURISTIC);
	est.calc_initial_guess_q0(q0, CSM::InitialStateDistributionInitialisationMethod::FROM_DATA);
	const double error = est.estimate(pi, q0);
	ASSERT_TRUE(error < 6.);
	for (size_t k = 0; k < dim * dim; ++k) {
		ASSERT_NEAR(0.25, q0[k], 2E-5) << k;
	}
	Eigen::MatrixXd expected_pi(dim, dim * dim);
	expected_pi.setZero();
	expected_pi(1, 0) = 1; // (0, 0) -> (0, 1)
	expected_pi(1, 1) = 1; // (0, 1) -> (1, 1)
	expected_pi(0, 2) = 1; // (1, 0) -> (0, 0)
	expected_pi(0, 3) = 1; // (1, 1) -> (1, 0)
	ASSERT_NEAR(0, (expected_pi - pi).norm(), 1E-12);
}

TEST(CSM, LongitudinalMemoryShortTraj) {
	const size_t dim = 2;
	ObservedDiscreteData data(dim);
	const size_t T = 2;
	const size_t ntraj = 4;
	const unsigned int memory = 2;
	data.ltrajs.init_rectangular(ntraj, T);
	data.ltimes.init_rectangular(ntraj, T);

	for (size_t q = 0; q < ntraj; ++q) {
		for (size_t t = 0; t < T; ++t) {
			data.ltimes(q, t) = static_cast<double>(t);
		}
	}
	data.ltrajs(0, 0) = 0;
	data.ltrajs(0, 1) = 0;
	data.ltrajs(1, 0) = 0;
	data.ltrajs(1, 1) = 1;
	data.ltrajs(2, 0) = 1;
	data.ltrajs(2, 1) = 0;
	data.ltrajs(3, 0) = 1;
	data.ltrajs(3, 1) = 1;

	CSM est(data, CSMParams(memory, 1, 0, 0, nullptr));
	ASSERT_EQ(dim, est.dim());

	Eigen::MatrixXd pi;
	Eigen::VectorXd q0;
	est.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::HEURISTIC);
	est.calc_initial_guess_q0(q0, CSM::InitialStateDistributionInitialisationMethod::FROM_DATA);
	const double error = est.estimate(pi, q0);
	ASSERT_TRUE(error < 6.);
	Eigen::VectorXd init_distr(dim);
	CSMUtils::reduce(q0.data(), Markov::calc_state_dim(dim, memory), init_distr.data(), dim, 0);
	ASSERT_NEAR(0.5, init_distr[0], 1E-12);
	ASSERT_NEAR(0.5, init_distr[1], 1E-12);
}

TEST(CSM, AnalyticalConfidenceIntervals) {
	ObservedDiscreteData data;
	data.times = { 2000, 2001, 2007 };
	data.nbr_surveys = Eigen::Vector3d( 800, 400, 808 );
	data.probs.resize(3, 3);
	data.probs.col(0) << Eigen::Vector3d(0.11, 0.37, 0.52);
	data.probs.col(1) << Eigen::Vector3d(0.2, 0.3, 0.5);
	data.probs.col(2) << Eigen::Vector3d(0.3, 0.35, 0.35);
	const unsigned int dim = 3;
	CSM csm(data, CSMParams(0, 1, dim, 0, nullptr));
	const unsigned int extrap_T = 20;
	Eigen::MatrixXd extrap_probs(dim, extrap_T);
	Eigen::MatrixXd extrap_probs_lower(dim, extrap_T);
	Eigen::MatrixXd extrap_probs_upper(dim, extrap_T);
	Eigen::MatrixXd pi(dim, dim);
	Eigen::VectorXd q0(dim);
	csm.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::HEURISTIC);
	csm.calc_initial_guess_q0(q0, CSM::InitialStateDistributionInitialisationMethod::FROM_DATA);
	/*const double fit_error = */csm.estimate(pi, q0);
	//std::cout << "Fit error: " << fit_error << std::endl;
	csm.extrapolate_analytic_confidence_intervals(pi, q0, extrap_probs, extrap_probs_lower, extrap_probs_upper, 0.95);
	/*std::cout << "Extrapolated probabilities:" << std::endl;
	std::cout << extrap_probs << std::endl;
	std::cout << "Confidence intervals:" << std::endl;
	std::cout << (extrap_probs_upper - extrap_probs_lower) << std::endl;*/
	for (unsigned int i = 0; i < extrap_T; ++i) {
		for (unsigned int k = 0; k < dim; ++k) {
			ASSERT_LE(extrap_probs_upper(k, i), 1.) << i << " " << k;
			ASSERT_GT(extrap_probs_upper(k, i), extrap_probs(k, i)) << i << " " << k;
			ASSERT_LT(extrap_probs_lower(k, i), extrap_probs(k, i)) << i << " " << k;
			ASSERT_GE(extrap_probs_lower(k, i), 0.) << i << " " << k;
		}
	}
}

TEST(CSM, AnalyticalConfidenceIntervalsMMA) {
	ObservedDiscreteData data;
	data.times = { 2000, 2001, 2007 };
	data.nbr_surveys = Eigen::Vector3d(800, 400, 808);
	data.probs.resize(3, 3);
	data.probs.col(0) << Eigen::Vector3d(0.11, 0.37, 0.52);
	data.probs.col(1) << Eigen::Vector3d(0.2, 0.3, 0.5);
	data.probs.col(2) << Eigen::Vector3d(0.3, 0.35, 0.35);
	const unsigned int dim = 3;
	CSM csm(data, CSMParams(0, 1, dim, 1e-3, std::make_shared<CSMRegulariserNearestNeighbours>(1)));
	const unsigned int extrap_T = 20;
	Eigen::MatrixXd extrap_probs(dim, extrap_T);
	Eigen::MatrixXd extrap_probs_lower(dim, extrap_T);
	Eigen::MatrixXd extrap_probs_upper(dim, extrap_T);
	Eigen::MatrixXd pi(dim, dim);
	Eigen::VectorXd q0(dim);
	csm.calc_initial_guess_pi(pi, CSM::TransitionMatrixInitialisationMethod::HEURISTIC);
	csm.calc_initial_guess_q0(q0, CSM::InitialStateDistributionInitialisationMethod::FROM_DATA);
	/*const double fit_error = */csm.estimate(pi, q0);
	//std::cout << "Fit error: " << fit_error << std::endl;
	csm.extrapolate_analytic_confidence_intervals(pi, q0, extrap_probs, extrap_probs_lower, extrap_probs_upper, 0.95);
	/*std::cout << "Extrapolated probabilities:" << std::endl;
	std::cout << extrap_probs << std::endl;
	std::cout << "Confidence intervals:" << std::endl;
	std::cout << (extrap_probs_upper - extrap_probs_lower) << std::endl;*/
	for (unsigned int i = 0; i < extrap_T; ++i) {
		for (unsigned int k = 0; k < dim; ++k) {
			ASSERT_LE(extrap_probs_upper(k, i), 1.) << i << " " << k;
			ASSERT_GT(extrap_probs_upper(k, i), extrap_probs(k, i)) << i << " " << k;
			ASSERT_LT(extrap_probs_lower(k, i), extrap_probs(k, i)) << i << " " << k;
			ASSERT_GE(extrap_probs_lower(k, i), 0.) << i << " " << k;
		}
	}
}

TEST(CSM, AnalyticalConfidenceIntervals2) {
	const unsigned int dim = 3;
	const unsigned int extrap_T = 20;
	ObservedDiscreteData data;
	data.times = { 2000, 2001, 2007 };
	data.nbr_surveys = Eigen::Vector3d(800, 400, 808);
	data.probs.resize(3, 3);
	data.probs.col(0) << Eigen::Vector3d(0.11, 0.37, 0.52);
	data.probs.col(1) << Eigen::Vector3d(0.2, 0.3, 0.5);
	data.probs.col(2) << Eigen::Vector3d(0.2, 0.4, 0.4);
	CSM csm(data, CSMParams(0, 1, dim, 0, nullptr));	
	
	Eigen::MatrixXd extrap_probs(dim, extrap_T);
	Eigen::MatrixXd extrap_probs_lower(dim, extrap_T);
	Eigen::MatrixXd extrap_probs_upper(dim, extrap_T);
	
	Eigen::MatrixXd pi(dim, dim);
	pi.col(0) = Eigen::Vector3d(1.0007064294013489e-14, 0.99999999999998002, 1.0002844601095225e-14);
	pi.col(1) = Eigen::Vector3d(0.42200172363801114, 0.51754021490546642, 0.060458061456522411);
	pi.col(2) = Eigen::Vector3d(0.084734447964259094, 1.0000000014547026e-14, 0.91526555203573101);
	const Eigen::VectorXd q0 = Eigen::Vector3d(0.10980830464661225, 0.36966792528099351, 0.52052377007239437);
	
	csm.extrapolate_analytic_confidence_intervals(pi, q0, extrap_probs, extrap_probs_lower, extrap_probs_upper, 0.95);
	/*std::cout << "Extrapolated probabilities:" << std::endl;
	std::cout << extrap_probs << std::endl;
	std::cout << "Confidence intervals:" << std::endl;
	std::cout << (extrap_probs_upper - extrap_probs_lower) << std::endl;*/
	for (unsigned int i = 0; i < extrap_T; ++i) {
		for (unsigned int k = 0; k < dim; ++k) {
			ASSERT_LE(extrap_probs_upper(k, i), 1.) << i << " " << k;
			ASSERT_GT(extrap_probs_upper(k, i), extrap_probs(k, i)) << i << " " << k;
			ASSERT_LT(extrap_probs_lower(k, i), extrap_probs(k, i)) << i << " " << k;
			ASSERT_GE(extrap_probs_lower(k, i), 0.) << i << " " << k;
		}
	}
}

TEST(CSM, transition_matrix_initialisation_method_from_string) {
	ASSERT_EQ(CSM::TransitionMatrixInitialisationMethod::FROM_TRAJECTORIES, CSM::transition_matrix_initialisation_method_from_string("FROM_TRAJECTORIES"));
	ASSERT_EQ(CSM::TransitionMatrixInitialisationMethod::HEURISTIC, CSM::transition_matrix_initialisation_method_from_string("HEURISTIC"));
	ASSERT_EQ(CSM::TransitionMatrixInitialisationMethod::IDENTITY, CSM::transition_matrix_initialisation_method_from_string("IDENTITY"));
	ASSERT_EQ(CSM::TransitionMatrixInitialisationMethod::MAX_ENTROPY, CSM::transition_matrix_initialisation_method_from_string("MAX_ENTROPY"));
}

TEST(CSM, initial_state_distribution_initialisation_method_from_string) {
	ASSERT_EQ(CSM::InitialStateDistributionInitialisationMethod::FROM_DATA, CSM::initial_state_distribution_initialisation_method_from_string("FROM_DATA"));
	ASSERT_EQ(CSM::InitialStateDistributionInitialisationMethod::MAX_ENTROPY, CSM::initial_state_distribution_initialisation_method_from_string("MAX_ENTROPY"));
}
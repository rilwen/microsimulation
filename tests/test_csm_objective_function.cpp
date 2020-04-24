#include <gtest/gtest.h>
#include "core/csm_objective_function.hpp"
#include "core/csm.hpp"
#include "core/csm_utils.hpp"
#include "core/observed_discrete_data.hpp"

// Test numerically the gradient of f
static void test_gradient(const char* message, averisera::CSMObjectiveFunction& f, std::vector<double>& x, const std::vector<double>& actual_gradient, const unsigned int arg_dim, const double tol) {
	std::vector<double> other_grad;
	const double h = 1E-7;
	for (unsigned int k = 0; k < arg_dim; ++k) {
		const double orig_x_k = x[k];
		const double x_k_up = orig_x_k + h;
		const double x_k_lo = x_k_up - 2 * h; // Ensure that x_k_up and x_k_lo differ by an exactly representable number
		const double dx = x_k_up - x_k_lo; //... which we calculate here and will use to compute the gradient instead of 2*h
		x[k] = x_k_up;
		const double y_up = f.value(x, other_grad, true);
		x[k] = x_k_lo;
		const double y_lo = f.value(x, other_grad, true);
		const double expected_grad_k = (y_up - y_lo) / dx;
		EXPECT_NEAR(expected_grad_k, actual_gradient[k], tol) << message << ": " << k; 
		x[k] = orig_x_k;
	}
}

// Test with pi which does not fit the input data
TEST(CSMObjectiveFunction, WithError) {
	const unsigned int T = 3;
	const unsigned int dim = 2;
	averisera::ObservedDiscreteData data(dim, T);
	Eigen::MatrixXd& p = data.probs;
	p.col(0) = Eigen::Vector2d(0.8, 0.2);
	p.col(1) = Eigen::Vector2d(0.25, 0.75);
	p.col(2) = Eigen::Vector2d(0.8, 0.2);

	averisera::CSMObjectiveFunction f_init_state(data, averisera::CSMParams(0, 1.0, dim, 0, nullptr));
	const Eigen::Map<const Eigen::MatrixXd> f_probs(&f_init_state.probs()[0], dim, T);
	ASSERT_EQ((p - f_probs).norm(), 0.0);
	ASSERT_EQ(6u, f_init_state.arg_dim());

	std::vector<double> x_no_init_state(dim * dim);
	std::vector<double> grad_no_init_state(dim * dim);
	x_no_init_state[0] = 0; x_no_init_state[1] = 1; x_no_init_state[2] = 1; x_no_init_state[3] = 0;

	const unsigned int arg_dim_init_state = dim * dim + dim;
	std::vector<double> x_init_state(x_no_init_state); // make a copy
	x_init_state.resize(arg_dim_init_state);
	for (unsigned int k = 0; k < dim; ++k)  {
		x_init_state[dim*dim + k] = p.col(0)[k]; // copy the initial state
	}
	std::vector<double> grad_init_state(arg_dim_init_state);
	std::vector<double> grad_init_state_wjac(arg_dim_init_state);
	std::vector<double> jacobian_init_state(arg_dim_init_state * arg_dim_init_state);

	const double actual_value_init_state = f_init_state.value(x_init_state, grad_init_state, true);
	const double actual_value_init_state_wjac = f_init_state.value(x_init_state, grad_init_state_wjac, jacobian_init_state, true);
	const double expected_value = (0.25 * log(0.25 / 0.2) + 0.75 * log(0.75 / 0.8)); // do not divide by T because weights are not normalised
	EXPECT_NEAR(expected_value, actual_value_init_state, 1E-16);
	EXPECT_NEAR(expected_value, actual_value_init_state_wjac, 1E-16);
	for (unsigned int i = 0; i < arg_dim_init_state; ++i) {
		EXPECT_NEAR(grad_init_state[i], grad_init_state_wjac[i], 1E-16) << i;
		for (unsigned int j = 0; j < i; ++j) {
			EXPECT_NEAR(jacobian_init_state[i * arg_dim_init_state + j], jacobian_init_state[j * arg_dim_init_state + i], 1E-16) << i << " " << j;
		}
	}

	Eigen::MatrixXd pi(dim, dim);
	std::copy(x_no_init_state.begin(), x_no_init_state.end(), pi.data());

	// Test extrapolation
	const unsigned int new_T = T + 2;
	Eigen::MatrixXd extrap(dim, new_T);
	averisera::CSMUtils::extrapolate(pi, p.col(0), extrap);
	for (unsigned int t = 0; t < new_T; ++t) {
		// pi matrix is [0 1;1 0], so extrapolated columns with even indices will be equal to p.col(0)
		if (t % 2 == 0) {
			ASSERT_NEAR(0, (extrap.col(t) - p.col(0)).norm(), 1E-15);
		} else {
			// if they're even, the coefficients will be swapped
			ASSERT_NEAR(p(1, 0), extrap(0, t), 1E-15);
			ASSERT_NEAR(p(0, 0), extrap(1, t), 1E-15) << t;
		}
	}

	// Test numerically the gradient
	// Set "random pi"
	x_no_init_state[0] = 0.2; x_no_init_state[1] = 0.8; x_no_init_state[2] = 0.9; x_no_init_state[3] = 0.1;
	std::copy(x_no_init_state.begin(), x_no_init_state.end(), x_init_state.begin()); // copy this to beginning of the longer argument vector
	x_init_state[4] = 0.23; x_init_state[5] = 0.77; // set "random initial state"
	//  recalculate gradients
	f_init_state.value(x_init_state, grad_init_state, true);
	test_gradient("With init state", f_init_state, x_init_state, grad_init_state, dim * dim + dim, 4E-9);	
}

// Test with pi which matches the input probabilities
TEST(CSMObjectiveFunction, NoError) {
	const unsigned int T = 4;
	const unsigned int dim = 3;
	averisera::ObservedDiscreteData data(dim, T);
	Eigen::MatrixXd& p = data.probs;
	p.col(0) = Eigen::Vector3d(0.26, 0.41, 0.33);
	p.col(1) = Eigen::Vector3d(0.245, 0.4445, 0.3105);
	p.col(2) = Eigen::Vector3d(0.23375, 0.471125, 0.295125);
	p.col(3) = Eigen::Vector3d(0.225313, 0.491656, 0.283031);

	Eigen::MatrixXd expected_pi(dim, dim);
	expected_pi.col(0) = Eigen::Vector3d(0.8, 0.1, 0.1);
	expected_pi.col(1) = Eigen::Vector3d(0.05, 0.9, 0.05);
	expected_pi.col(2) = Eigen::Vector3d(0.05, 0.15, 0.8);

	const unsigned int memory = 0;
	averisera::CSMObjectiveFunction f(data, averisera::CSMParams(memory, 1.0, dim, 0, nullptr));

	std::vector<double> pi_coeffs(dim*dim + dim);
	std::vector<double> grad(dim * dim + dim);
	averisera::CSMUtils::copy_probabilities(expected_pi, dim, p.col(0), pi_coeffs, memory);
	ASSERT_NEAR(0.0, f.value(pi_coeffs, grad, true), 1E-10);

	//// Turn on this test only for squared errors
	//for (unsigned int k = 0; k < dim*dim; ++k) {
	//	EXPECT_NEAR(0.0, grad[k], 0) << k;
	//}
}

// Test with identity pi and non-zero memory - just to see that it works
TEST(CSMObjectiveFunction, NoErrorMemory) {
	const unsigned int T = 2;
	const unsigned int dim = 3;
	averisera::ObservedDiscreteData data(dim, T);
	for (unsigned int t = 0; t < T; ++t) {
		data.probs.col(t) = Eigen::Vector3d(0.26, 0.41, 0.33);
	}
	
	const unsigned int memory = 1;
	const unsigned int arg_dim = averisera::CSM::calc_arg_dim(dim, memory);
	averisera::CSMObjectiveFunction f(data, averisera::CSMParams(memory, 1.0, 0, 0, nullptr));
	std::vector<double> x(arg_dim, 0.0);
	
	std::vector<double> grad(arg_dim);
	// Prepare the conditional transition probabilities (== pi matrix) and the initial state
	for (unsigned int m = 0; m < dim; ++m) {
		for (unsigned int k = 0; k < dim; ++k) {
			// P(X_t = k | X_t-1 = l && X_t-2 = m) = delta_kl
			x[(m * dim + k) * dim + k] = 1.0;
			// Assume zero correlation: P(X_0 = k && P_-1 = l) = P(k) * P(l)
			x[dim*dim*dim + m * dim + k] = data.probs.col(0)[k] * data.probs.col(0)[m];
		}		
	}
	
	ASSERT_NEAR(0, f.value(x, grad, true), 1E-15);
	test_gradient("Memory == 1", f, x, grad, arg_dim, 3E-9);

	grad.resize(0);
	ASSERT_NEAR(0, f.value(x, grad, true), 1E-15);	
}

// Test with identity pi and non-zero memory of size 2 - just to see that it works
TEST(CSMObjectiveFunction, NoErrorMemoryTwo) {
	const unsigned int T = 2;
	const unsigned int dim = 3;
	averisera::ObservedDiscreteData data(dim, T);
	Eigen::MatrixXd& p = data.probs;
	p.col(0) = Eigen::Vector3d(0.26, 0.41, 0.33);
	p.col(1) = Eigen::Vector3d(0.26, 0.41, 0.33);
	
	const unsigned int memory = 2;
	averisera::CSMObjectiveFunction f(data, averisera::CSMParams(memory, 1, 0, 0, nullptr));
	const unsigned int arg_dim = averisera::CSM::calc_arg_dim(dim, memory);
	std::vector<double> x(arg_dim, 0.0);
	std::vector<double> grad(arg_dim);
	// Prepare the conditional transition probabilities (== pi matrix) and the initial state
	for (unsigned int n = 0; n < dim; ++n) {
		for (unsigned int m = 0; m < dim; ++m) {
			for (unsigned int k = 0; k < dim; ++k) {
				// P(X_t = k | X_t-1 = l && X_t-2 = m && X_t-3 = n) = delta_kl
				x[((n * dim + m) * dim + k) * dim + k] = 1.0;
				// Assume zero correlation: P(X_0 = k && P_-1 = l) = P(k) * P(l)
				x[dim*dim*dim*dim + (n * dim + m) * dim + k] = p.col(0)[n] * p.col(0)[k] * p.col(0)[m];
			}		
		}
	}

	ASSERT_NEAR(0, f.value(x, grad, true), 1E-15);
	test_gradient("Memory == 2", f, x, grad, arg_dim, 3E-9);

	grad.resize(0);
	ASSERT_NEAR(0, f.value(x, grad, true), 1E-15);
}

// Test if the value computed is accurate for longer time periods
TEST(CSMObjectiveFunction, ValueStability) {
	const unsigned int T = 200;
	const unsigned int dim = 2;

	averisera::ObservedDiscreteData data(dim, T);
	Eigen::MatrixXd& p = data.probs;
	p.col(0) = Eigen::Vector2d(0.8, 0.2);
	for (unsigned int t = 1; t < T; ++t) {
		p.col(t) = Eigen::Vector2d(1.0, 0.0);
	}

	averisera::CSMObjectiveFunction f(data, averisera::CSMParams(0, 1, 0, 0, nullptr));

	std::vector<double> pi_coeffs(dim * dim + dim);
	std::vector<double> grad(dim * dim + dim);
	// Immediate transition to 1st value of X
	pi_coeffs[0] = 1; pi_coeffs[1] = 0; pi_coeffs[2] = 1; pi_coeffs[3] = 0;
	pi_coeffs[4] = 0.8; pi_coeffs[5] = 0.2;

	const double actual_value = f.value(pi_coeffs, grad, true);
	const double expected_value = 0;
	ASSERT_NEAR(expected_value, actual_value, 1E-16);
}

// TODO: test value without normalization term
// TODO: test value with jacobian

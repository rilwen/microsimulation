#include <gtest/gtest.h>
#include "core/csm_regulariser_entropy.hpp"
#include "core/csm_workspace.hpp"

using namespace averisera;

TEST(CSMRegulariserEntropy, lowest) {
	const unsigned int dim = 2;
	const double eps = 1e-15;
	std::vector<double> x(dim * (dim + 1), eps);

	x[0] = 1 - eps;
	x[2] = 1 - eps;
	x[4] = 1 - eps;

	CSMWorkspaceNoData<0> wksp(1, dim, 0);
	wksp.set_calibrated_parameters(x, false);

	CSMRegulariserEntropy regulariser(1, 1);

	regulariser.operator()(wksp);

	const double actual = wksp.regularisation_term.val();
	ASSERT_NEAR(0, actual, 1e-12);
}

TEST(CSMRegulariserEntropy, uniform_minus_minus) {
	const unsigned int dim = 2;
	std::vector<double> x(dim * (dim + 1), 0.5);

	CSMWorkspaceNoData<0> wksp(1, dim, 0);
	wksp.set_calibrated_parameters(x, false);

	CSMRegulariserEntropy regulariser(-1, -1);

	regulariser.operator()(wksp);

	const double actual = wksp.regularisation_term.val();
	const double expected = - (static_cast<double>(x.size()) * 0.5 * std::log(0.5));
	ASSERT_NEAR(expected, actual, 1e-15);
}

TEST(CSMRegulariserEntropy, uniform_plus_minus) {
	const unsigned int dim = 2;
	std::vector<double> x(dim * (dim + 1), 0.5);

	CSMWorkspaceNoData<0> wksp(1, dim, 0);
	wksp.set_calibrated_parameters(x, false);

	CSMRegulariserEntropy regulariser(1, -1);

	regulariser.operator()(wksp);

	const double actual = wksp.regularisation_term.val();
	const double expected = (dim * dim - dim) * 0.5 * std::log(0.5);
	ASSERT_NEAR(expected, actual, 1e-15);
}
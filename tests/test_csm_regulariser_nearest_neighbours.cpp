// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/csm_regulariser_nearest_neighbours.hpp"
#include "core/csm_workspace.hpp"

using namespace averisera;

TEST(CSMRegulariserNearestNeighbours, test) {
	const unsigned int dim = 2;
	const unsigned int memory = 1;
	const unsigned int state_dim = dim * dim;
	std::vector<double> x(state_dim * (dim + 1), NAN);

	// (0, 0) ->
	x[0] = 0.45;
	x[1] = 0.55;
	// (0, 1) ->
	x[2] = 0.1;
	x[3] = 0.9;
	// (1, 0) ->
	x[4] = 0.3;
	x[5] = 0.7;
	// (1, 1) ->
	x[6] = 0.5;
	x[7] = 0.5;

	CSMWorkspaceNoData<0> wksp(1, dim, memory);
	wksp.set_calibrated_parameters(x, false);

	CSMRegulariserNearestNeighbours regulariser(0);

	regulariser.operator()(wksp);

	const double actual = wksp.regularisation_term.val();
	const double expected = x[1] * x[1] + x[2] * x[2] + x[5] * x[5] + x[6] * x[6];
	ASSERT_NEAR(expected, actual, 1e-15);
}
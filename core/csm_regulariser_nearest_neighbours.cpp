/*
(C) Averisera Ltd 2019
*/
#include "csm_regulariser_nearest_neighbours.hpp"
#include "csm_workspace.hpp"

namespace averisera {
	CSMRegulariserNearestNeighbours::CSMRegulariserNearestNeighbours(unsigned int max_dist_not_penalised)
		: max_dist_not_penalised_(max_dist_not_penalised) {}

	void CSMRegulariserNearestNeighbours::operator()(CSMWorkspaceNoData<0>& wksp) const {
		calc(wksp);
	}

	void CSMRegulariserNearestNeighbours::operator()(CSMWorkspaceNoData<1>& wksp) const {
		calc(wksp);
	}

	template <unsigned int L> void CSMRegulariserNearestNeighbours::calc(CSMWorkspaceNoData<L>& wksp) const {
		typename CSMWorkspaceNoData<L>::ad_scalar_t sum(0.);
		const unsigned int state_dim_div = wksp.state_dim / wksp.dim; // e.g. 3
		for (unsigned int from = 0; from < wksp.state_dim; ++from) {
			const unsigned int last_cat = from % wksp.dim; // e.g. 1
			const unsigned int all_cats_but_oldest = from % state_dim_div; // e.g. 2
			for (unsigned int next_cat = 0; next_cat < wksp.dim; ++next_cat) {
				const unsigned int next_state = all_cats_but_oldest * wksp.dim + next_cat;
				if (static_cast<unsigned int>(std::abs(static_cast<int>(last_cat - next_cat))) > max_dist_not_penalised_) {
					const auto pi_term = wksp.pi_expanded[from * wksp.state_dim + next_state];
					sum += pi_term * pi_term;
				}				
			}
		}
		wksp.regularisation_term = sum;
	}
}
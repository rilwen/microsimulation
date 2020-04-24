/*
(C) Averisera Ltd 2014
*/
#include "dirichlet_distribution.hpp"
#include "preconditions.hpp"
#include <cassert>

namespace averisera {

	static void check_alphas(const std::vector<double>& alphas) {
		check_that(alphas.size() >= 2, "At least two alphas required");
		check_that(alphas.begin(), alphas.end(), [](double alpha){return alpha > 0; }, "Alpha must be positive");
	}

	DirichletDistribution::DirichletDistribution(size_t N, double alpha) 
		: _alphas(N, alpha)
	{
		check_alphas(_alphas);
	}

	DirichletDistribution::DirichletDistribution(const std::vector<double>& alphas)
		: _alphas(alphas)
	{
		check_alphas(_alphas);
	}

	DirichletDistribution::DirichletDistribution(std::vector<double>&& alphas)
		: _alphas(alphas)
	{
		check_alphas(_alphas);
	}

	void DirichletDistribution::set_alpha(size_t idx, double alpha) {
		assert(alpha > 0);
		_alphas[idx] = alpha;
	}

	double DirichletDistribution::get_alpha(size_t idx) const {
		return _alphas[idx];
	}
}
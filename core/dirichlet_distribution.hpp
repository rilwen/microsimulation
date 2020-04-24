/*
(C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_DIRICHLET_DISTRIBUTION_H
#define __AVERISERA_DIRICHLET_DISTRIBUTION_H

#include <algorithm>
#include <random>
#include <vector>

namespace averisera {
	// Dirichlet distribution
	class DirichletDistribution {
	public:		
		// Create D.d. with constant alpha (1 by default).
		// alpha: Positive number
		DirichletDistribution(size_t N, double alpha = 1.0);

		DirichletDistribution(const std::vector<double>& alphas);

		DirichletDistribution(std::vector<double>&& alphas);

		size_t size() const {
			return _alphas.size();
		}

		void set_alpha(size_t idx, double alpha);
		double get_alpha(size_t idx) const;

		// Sample from distribution
		// p: Vector resized at exit to size()
		// URNG: STL uniform RNG type
		template <class URNG> void sample(URNG& rng, std::vector<double>& p) const {
			const size_t N = size();
			p.resize(N);			
			double sum = 0;
			for (size_t i = 0; i < N; ++i) {
				std::gamma_distribution<double> gamma(_alphas[i], 1.0);
				const double y = gamma(rng);
				p[i] = y;
				sum += y;
			}
			std::transform(p.begin(), p.end(), p.begin(), [sum](double y){ return y / sum; });
		}
	private:
		std::vector<double> _alphas;
	};
}

#endif

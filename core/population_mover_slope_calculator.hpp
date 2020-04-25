#pragma once
#include <Eigen/Core>
#include <vector>

namespace averisera {
    /**
      Given a distribution distr[i], calculate coefficients a[i] and b[i] such that

      distr[i] = int_0^1 (a[i] + b[i] * u) du

      and b[i] < 0 for i < from_idx, b[i] > 0 for i > from_idx and b[i] == 0 for i == from_idx.

      When i == category index and u == percentile of individual within a category, this
      algorithm is used to update a population with a Markov model transition matrix.

      @see PopulationMover
     */
	class PopulationMoverSlopeCalculator {
	public:
		PopulationMoverSlopeCalculator(double tolerance);
		void calculate(Eigen::Ref<const Eigen::VectorXd> distr, size_t from_idx, std::vector<double>& a, std::vector<double>& b) const;
	private:
		double tolerance_;

		void calc_b(Eigen::Ref<const Eigen::VectorXd> distr, const std::vector<double>& a, std::vector<double>& b) const;
	};
}

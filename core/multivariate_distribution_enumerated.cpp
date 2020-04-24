#include "multivariate_distribution_enumerated.hpp"
#include "preconditions.hpp"

namespace averisera {
	MultivariateDistributionEnumerated::MultivariateDistributionEnumerated(const Eigen::MatrixXd& values, const std::vector<double>& probs)
		: values_(values), probs_(0, probs), dim_(static_cast<size_t>(values.cols())) {
		check_equals(probs.size(), static_cast<size_t>(values.rows()), "MultivariateDistributionEnumerated: different number of probabilities and distribution values");
		check_that(values.cols() > 0, "MultivariateDistributionEnumerated: need at least 1 column");
	}

	template <class V> void MultivariateDistributionEnumerated::draw_impl(RNG& rng, V v) const {
		const size_t idx = static_cast<size_t>(probs_.draw(rng));
		v = values_.row(idx);
	}
}

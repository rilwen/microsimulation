#pragma once
#include "discrete_distribution.hpp"
#include "multivariate_distribution.hpp"
#include <Eigen/Core>

namespace averisera {
	/** Draws random row from a matrix with specified probabilities 
	TODO: finish it */
	class MultivariateDistributionEnumerated : public MultivariateDistribution {
	public:
		/** @throw std::domain_error If values.rows() != probs.size(), probs.size() == 0 or values.cols() == 0 */
		MultivariateDistributionEnumerated(const Eigen::MatrixXd& values, const std::vector<double>& probs);

		size_t dim() const override {
			return dim_;
		}

		void draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const override {
			draw_impl(rng, x);
		}

		void draw_noncont(RNG& rng, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> x) const override {
			draw_impl(rng, x);
		}
	private:
		Eigen::MatrixXd values_;
		DiscreteDistribution probs_;
		size_t dim_;

		template <class V> void draw_impl(RNG& rng, V v) const;
	};
}

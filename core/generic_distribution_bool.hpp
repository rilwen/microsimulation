#pragma once
#include "generic_distribution.hpp"

namespace averisera {
	/** Most basic distribution of a boolean random variable. Bool values are assumed to be ordered  */
	class GenericDistributionBool : public GenericDistribution<bool> {
	public:
		/** @throw std::out_of_range If p not in [0, 1] */
		GenericDistributionBool(double p);

		bool random(RNG& rng) const override;

		GenericDistributionBool* conditional(bool left, bool right) const override;

		double range_prob2(bool x1, bool x2) const override;

		bool icdf_generic(double p) const override;

		bool lower_bound() const override;

		bool upper_bound() const override;
	private:
		double _p; /**< P(true) */
	};
}

// (C) Averisera Ltd 2014-2020
#pragma once
#include "sampling_distribution.hpp"
#include "generic_distribution.hpp"

namespace averisera {
	/** Wrapper around GenericDistribution reference converting it into a SamplingDistribution */
	template <class T> class SamplingDistributionFromGenericRef: public SamplingDistribution {
	public:
		SamplingDistributionFromGenericRef(const GenericDistribution<T>& gen_distr)
			: gd_(gen_distr) {}

		double draw(RNG& rng) const override {
			return static_cast<double>(gd_.random(rng));
		}
	private:
		const GenericDistribution<T>& gd_;
	};
}

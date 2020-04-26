// (C) Averisera Ltd 2014-2020
#pragma once

namespace averisera {
	class RNG;

	/** Real-valued distribution we can sample from */
	class SamplingDistribution {
	public:
		virtual ~SamplingDistribution() {}

		/** Draw a random number from the distribution. */
		virtual double draw(RNG& rng) const = 0;
	};
}

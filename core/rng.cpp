/*
 * (C) Averisera Ltd 2015
 */
#include "rng.hpp"

namespace averisera {
    RNG::~RNG() {}

	bool RNG::flip(const double p) {
		assert(p >= 0 && p <= 1);
		const double u = next_uniform();
		return u < p;
	}
}

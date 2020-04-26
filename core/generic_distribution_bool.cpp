// (C) Averisera Ltd 2014-2020
#include "generic_distribution_bool.hpp"
#include "rng.hpp"

namespace averisera {
	GenericDistributionBool::GenericDistributionBool(double p)
		: _p(p) {
		if (_p < 0 || _p > 1) {
			throw std::out_of_range("GenericDistributionBool: p out of range");
		}
	}

	bool GenericDistributionBool::random(RNG& rng) const {
		return rng.next_uniform() > _p;
	}

	GenericDistributionBool* GenericDistributionBool::conditional(bool left, bool right) const {
		if (!left && right && (_p < 1)) {
			return new GenericDistributionBool(0.0);
		} else {
			throw std::runtime_error("GenericDistributionBool: conditioning on zero probability region");
		}
	}

	double GenericDistributionBool::range_prob2(bool x1, bool x2) const {
		if (!x1 && x2) {
			return 1 - _p;
		} else {
			return 0;
		}
	}

	bool GenericDistributionBool::icdf_generic(double p) const {
		if (p < 0 || p > 1) {
			throw std::out_of_range("GenericDistributionBool: p out of range");
		}
		return p > 1 - _p;
	}

	bool GenericDistributionBool::lower_bound() const {
		return false;
	}

	bool GenericDistributionBool::upper_bound() const {
		return _p > 0 ? true : false;
	}
}

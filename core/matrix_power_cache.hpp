/*
(C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_MATRIX_POWER_CACHE_H
#define __AVERISERA_MATRIX_POWER_CACHE_H

#include <cassert>
#include <vector>

namespace averisera {
	// M: Matrix class, e.g. Eigen::MatrixXd, with a default constructor.
	template <class M> class MatrixPowerCache {
	public:
		// m: Matrix of class M
		MatrixPowerCache(const M& m) 
		: _max_power(1), _cache(1, m), _initialized(1, true) {}

		// Get m^q
		// q: q > 0
		const M& power(unsigned int q);

		// Return maximum q for which we have calculated m^q
		unsigned int max_power() const {
			return _max_power;
		}

		// Check if m^q is available in the cache
		// q: q > 0
		bool is_power_available(unsigned int q) const {
			assert(q > 0);
			if (q <= max_power()) {
				return _initialized[q - 1];
			} else {
				return false;
			}
		}
	private:
		// Resize storage to new size
		void resize_storage(size_t new_size);

		unsigned int _max_power;
		std::vector<M> _cache;
		std::vector<bool> _initialized; // _initialized[q] tells us if we have m^(q + 1) available
	};

	template <class M> const M& MatrixPowerCache<M>::power(unsigned int q) {
		assert(q > 0);		
		if (!is_power_available(q)) {
			resize_storage(q);
			assert(q > 1);
			const unsigned int l = q / 2;
			const unsigned int r = q - l;
			assert(l > 0);
			assert(r > 0);
			assert(l + r == q);
			const M& ml = power(l);
			const M& mr = power(r);
			_cache[q - 1].noalias() = ml * mr; // If using different linear algebra than Eigen, this probably needs to be generalized (traits?)
			_initialized[q - 1] = true;
			_max_power = std::max(q, _max_power);
		}
		return _cache[q - 1];
	}

	template <class M> void MatrixPowerCache<M>::resize_storage(size_t new_size) {
		assert(_cache.size() == _initialized.size());
		const size_t old_size = _cache.size();
		if (new_size > old_size) {
			_cache.resize(new_size);
			_initialized.resize(new_size);
			std::fill(_initialized.begin() + old_size, _initialized.end(), false);
		}
	}
}

#endif
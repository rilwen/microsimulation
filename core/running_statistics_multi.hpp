// (C) Averisera Ltd 2014-2020
#pragma once
#include "core/running_statistics.hpp"
#include "core/running_covariance.hpp"
#include <cassert>
#include <vector>

namespace averisera {
	/** Multidimensional running statistics
	@tparam V Type of collected values
	*/
	template <class V = double> class RunningStatisticsMulti {
	public:
		RunningStatisticsMulti()
			: RunningStatisticsMulti(0) {
		}

		RunningStatisticsMulti(size_t dim)
			: _dim(dim),
			_marginal_stats(dim),
			_covariances(flat_dim(dim)) {}

		RunningStatisticsMulti(RunningStatisticsMulti<V>&& other)
			: _dim(other._dim),
			_marginal_stats(std::move(other._marginal_stats)),
			_covariances(std::move(other._covariances)) {
			other._dim = 0;
		}

		RunningStatisticsMulti& operator=(RunningStatisticsMulti<V>&& other) {
			if (this != &other) {
				RunningStatisticsMulti<V> copy(std::move(other));
				swap(copy);
			}
			return *this;
		}

		void swap(RunningStatisticsMulti<V>& other) {
			std::swap(_dim, other._dim);
			_marginal_stats.swap(other._marginal_stats);
			_covariances.swap(other._covariances);
		}

		RunningStatisticsMulti(const RunningStatisticsMulti<V>&) = delete;
		RunningStatisticsMulti& operator=(const RunningStatisticsMulti<V>&) = delete;

		/** Add elements.
		@tparam V Type with .size() and operator[]
		*/
		template <class Vec> void add(const Vec& vec) {
			const size_t n = vec.size();
			for (size_t i = 0; i < n; ++i) {
				const V& x = vec[i];
				marginal(i).add(x);
				for (size_t j = 0; j < i; ++j) {
					covariance(i, j).add(x, vec[j]);
				}
			}
		}

		/** Add elements if all are not NaN.
		@tparam V Type with .size() and operator[]
		*/
		template <class Vec> void add_if_all_not_nan(const Vec& vec) {
			const size_t n = vec.size();
			for (size_t i = 0; i < n; ++i) {
				if (std::isnan(vec[i])) {
					return;
				}
			}
			for (size_t i = 0; i < n; ++i) {
				const V& x = vec[i];
				marginal(i).add(x);
				for (size_t j = 0; j < i; ++j) {
					covariance(i, j).add(x, vec[j]);
				}
			}
		}

		/** Add elements if all are finite.
		@tparam V Type with .size() and operator[]
		*/
		template <class Vec> void add_if_all_finite(const Vec& vec) {
			const size_t n = vec.size();
			for (size_t i = 0; i < n; ++i) {
				if (!std::isfinite(vec[i])) {
					return;
				}
			}
			for (size_t i = 0; i < n; ++i) {
				const V& x = vec[i];
				marginal(i).add(x);
				for (size_t j = 0; j < i; ++j) {
					covariance(i, j).add(x, vec[j]);
				}
			}
		}

		size_t dim() const {
			return _dim;
		}

		const RunningStatistics<V>& marginal(size_t i) const {
			assert(i < _dim);
			return _marginal_stats[i];
		}

		RunningStatistics<V>& marginal(size_t i) {
			assert(i < _dim);
			return _marginal_stats[i];
		}

		/** i > j */
		const RunningCovariance<V>& covariance(size_t i, size_t j) const {
			if (i < j) {
				return covariance(j, i);
			}				
			assert(i < _dim);
			const size_t k = flat_idx(i, j);
			return _covariances[k];
		}

		/** i > j */
		RunningCovariance<V>& covariance(size_t i, size_t j) {
			if (i < j) {
				return covariance(j, i);
			}
			assert(i < _dim);
			const size_t k = flat_idx(i, j);
			return _covariances[k];
		}
	private:
		size_t _dim;
		std::vector<RunningStatistics<V>> _marginal_stats;
		std::vector<RunningCovariance<V>> _covariances;

		static size_t flat_idx(size_t i, size_t j) {
			assert(i > j);
			return (i * (i - 1)) / 2 + j;
		}

		static size_t flat_dim(size_t dim) {
            if (dim > 1) {
                return flat_idx(dim - 1, dim - 2) + 1;
            } else {
                return 0;
            }
		}
	};

	template <class V> void swap(RunningStatisticsMulti<V>& l, RunningStatisticsMulti<V>& r) {
		l.swap(r);
	}
}

// (C) Averisera Ltd 2014-2020
#pragma once
#include "running_statistics.hpp"
#include <vector>

namespace averisera {
	/** Calculates covariance of two variables X and Y with incremental updates.
	@tparam T real number type */
	template <class T = double> class RunningCovariance {
	public:
		RunningCovariance();

		/** Add numbers */
		void add(T x, T y);

		/** Add if neither x nor y are NaN. Return whether the values were added or not. */
		bool add_if_not_nan(T x, T y);

		/** Mean of X */
		T meanX() const {
			return _x.mean();
		}

		/** Mean of Y */
		T meanY() const {
			return _y.mean();
		}

		/** Sample variance of X */
		T varianceX() const {
			return _x.variance();
		}

		/** Sample variance of Y */
		T varianceY() const {
			return _y.variance();
		}

		/** Sample covariance of X and Y */
		T covariance() const {
			return _C / (static_cast<T>(nbr_samples()) - 1.0);
		}

		/** Sample correlation of X and Y */
		T correlation() const;

		typedef typename RunningStatistics<T>::counter_t counter_t;

		/** Number of accumulated samples */
		counter_t nbr_samples() const {
			return _x.nbr_samples();
		}
	private:
		RunningStatistics<T> _x;
		RunningStatistics<T> _y;
		T _C; /** Co-moment C_n = sum_i=1^n (x_i - mean_x_n) * (y_i - mean_y_n) */
	};
}
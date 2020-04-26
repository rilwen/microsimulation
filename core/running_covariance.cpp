// (C) Averisera Ltd 2014-2020
#include "running_covariance.hpp"
#include <cmath>
#include <limits>

namespace averisera {
	template <class T> RunningCovariance<T>::RunningCovariance()
		: _C(std::numeric_limits<T>::quiet_NaN()) {

	}

	template <class T> void RunningCovariance<T>::add(T x, T y) {
		const T old_mean_x = _x.mean();
		_x.add(x);
		_y.add(y);
		if (_x.nbr_samples() == 1) {			
			if (std::isfinite(x) && std::isfinite(y)) {
				_C = 0.0;
			} else {
				_C = std::numeric_limits<T>::quiet_NaN();
			}
		} else {
			_C += (y - _y.mean()) * (x - old_mean_x);
		}				
	}

	template <class T> bool RunningCovariance<T>::add_if_not_nan(T x, T y) {
		if (!(std::isnan(x) || std::isnan(y))) {
			add(x, y);
			return true;
		} else {
			return false;
		}
	}

	template <class T> T RunningCovariance<T>::correlation() const {
		const T varX = varianceX();
		const T varY = varianceY();
		if (varX != 0 && varY != 0) {
			return covariance() / std::sqrt(varX * varY);
		} else {
			return 0;
		}
	}

	template class RunningCovariance<double>;
	template class RunningCovariance<long double>;
}
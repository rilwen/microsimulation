// (C) Averisera Ltd 2014-2020
#include "running_mean.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <numeric>

namespace averisera {
	template <class T> RunningMean<T>::RunningMean() {
		reset();
	}

	template <class T> void RunningMean<T>::add_finite(T x) {
		assert(!std::isinf(x));
		++_cnt;
		_m1 += (x - _m1) / static_cast<T>(_cnt);
	}

	template <class T> void RunningMean<T>::add(T x)
	{
		if (!std::isinf(x)) {
			add_finite(x);
		} else {
			if (std::isfinite(_m1)) {
				_m1 = x;
				++_cnt;
			} else if (std::isinf(_m1) && (std::signbit(_m1) == std::signbit(x))) {
				_m1 = x;
				++_cnt;
			} else {
				_m1 = std::numeric_limits<T>::quiet_NaN();
				++_cnt;
			}
		}
	}

	template <class T> bool RunningMean<T>::add_if_finite(T x) {
		if (std::isfinite(x)) {
			add(x);
			return true;
		} else {
			return false;
		}
	}

	template <class T> bool RunningMean<T>::add_if_not_nan(T x) {
		if (!std::isnan(x)) {
			add(x);
			return true;
		} else {
			return false;
		}
	}

	template <class T> void RunningMean<T>::add(const std::vector<T>& elems)
	{
		add(std::accumulate(elems.begin(), elems.end(), 0.0));
	}

	template <class T> bool RunningMean<T>::add_if_finite(const std::vector<T>& elems)
	{
		return add_if_finite(std::accumulate(elems.begin(), elems.end(), 0.0));
	}

	template <class T> T RunningMean<T>::mean() const {
		if (_cnt) {
			return _m1;
		} else {
			return std::numeric_limits<T>::quiet_NaN();
		}
	}

	template <class T> void RunningMean<T>::reset() {
		_m1 = 0.0;
		_cnt = 0;
	}


	template class RunningMean<double>;
	template class RunningMean<long double>;
}

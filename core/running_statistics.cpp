/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include "running_statistics.hpp"
#include "sacado_scalar.hpp"
#include <limits>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iostream>

// Use std::log, std::sqrt, etc. to handle long double properly

namespace averisera {
    using std::exp;
	template <class T> RunningStatistics<T>::RunningStatistics() 
	{
		_min = std::numeric_limits<T>::infinity();
		_max = - std::numeric_limits<T>::infinity();
		_cnt = 0;
		_m1 = _m2 = _m3 = _m4 = std::numeric_limits<T>::quiet_NaN();
	}

	template <class T> void RunningStatistics<T>::add(T x) 
	{
		if (std::isnan(x)) {
			_min = _max = _m1 = _m2 = _m3 = _m4 = x;
			++_cnt;
		} else {
			if (_cnt > 0) {
				if (!std::isinf(x)) {
					_min = std::min(_min, x);
					_max = std::max(_max, x);

					const T prev_cnt = fcnt();
					++_cnt;
					const T cnt = fcnt();
					const T dx = x - _m1;
					const T dx1 = dx / cnt;
					const T dx2 = dx1 * dx1;
					const T tmp = dx * dx1 * prev_cnt;
					_m1 += dx1;
					_m4 += tmp * dx2 * (cnt*cnt - 3 * cnt + 3) + 6 * dx2 * _m2 - 4 * dx1 * _m3;
					_m3 += tmp * dx1 * (cnt - 2) - 3 * dx1 * _m2;
					_m2 += dx * (x - _m1);// Kahan's algo
				} else {
					if (std::isfinite(_m1) ||
						(std::isinf(_m1) && (std::signbit(_m1) == std::signbit(x)))) {
						_m1 = x;
						_m2 = _m3 = _m4 = std::numeric_limits<T>::quiet_NaN();
						++_cnt;
					} else {
						_m1 = _m2 = _m3 = _m4 = std::numeric_limits<T>::quiet_NaN();
						++_cnt;
					}
				}
			} else {
				_min = _max = _m1 = x;
				_m2 = _m3 = _m4 = 0;
				++_cnt;
			}
		}
	}

	template <class T> bool RunningStatistics<T>::add_if_finite(T x) {
		if (std::isfinite(x)) {
			add(x);
			return true;
		} else {
			return false;
		}		
	}

	template <class T> bool RunningStatistics<T>::add_if_not_nan(T x) {
		if (!std::isnan(x)) {
			add(x);
			return true;
		} else {
			return false;
		}
	}

	template <class T> void RunningStatistics<T>::add(const std::vector<T>& elems)
	{
		add(std::accumulate(elems.begin(), elems.end(), T(0.0)));
	}

	template <class T> bool RunningStatistics<T>::add_if_finite(const std::vector<T>& elems)
	{
		return add_if_finite(std::accumulate(elems.begin(), elems.end(), T(0.0)));
	}

	template <class T> T RunningStatistics<T>::variance() const
	{
		return _m2 / (fcnt() - 1.0);
	}

	template <class T> T RunningStatistics<T>::standard_deviation() const
	{
		return std::sqrt(variance());
	}

	template <class T> T RunningStatistics<T>::skewness() const
	{
		return std::sqrt(fcnt()) * _m3 / std::pow(_m2, 1.5);
	}

	template <class T> T RunningStatistics<T>::kurtosis() const
	{
		return fcnt() * (_m4 / (_m2 * _m2)) - 3.0;
	}

    template class RunningStatistics<double>;
    template class RunningStatistics<long double>;
    template class RunningStatistics<adouble>;
}

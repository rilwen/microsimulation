// (C) Averisera Ltd 2014-2020
#pragma once

namespace averisera {
	/** Class which can store a value inside and return it when dereferenced. Used to implement *i++ in InputIterator */
	template <class T> class PostIncrementProxy {
	public:
		PostIncrementProxy(const T& v)
			: _val(v) {}

		T operator*() const {
			return _val;
		}
	private:
		T _val;
	};
}
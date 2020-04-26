// (C) Averisera Ltd 2014-2020
#pragma once

namespace averisera {

	template <class I> class IsDereferencable {
	public:
		IsDereferencable(I end)
			: _end(end) {}
		bool operator()(const I& iter) const {
			return iter != _end;
		}
	private:
		I _end;
	};	
}
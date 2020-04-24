#pragma once
#include <iosfwd>

namespace averisera {
	/** Remembers the position in an input stream when being created and restores it when being destroyed */
	class PositionKeeperIn {
	public:
		PositionKeeperIn(std::istream& stream);

		~PositionKeeperIn();
	private:
		std::istream& stream_;
		std::streampos pos_;
	};
}

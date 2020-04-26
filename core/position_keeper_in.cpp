// (C) Averisera Ltd 2014-2020
#include "position_keeper_in.hpp"
#include <istream>

namespace averisera {
	PositionKeeperIn::PositionKeeperIn(std::istream& stream)
		: stream_(stream), pos_(stream.tellg()) {}

	PositionKeeperIn::~PositionKeeperIn() {
		stream_.seekg(pos_);
	}
}

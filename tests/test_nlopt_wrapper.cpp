// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include <core/nlopt_wrap.hpp>

using namespace averisera;

TEST(NloptWrapper, names) {
	for (int i = 0; i < nlopt::NUM_ALGORITHMS; ++i) {
		std::cout << i << "\t" << nlopt::algorithm_name(static_cast<nlopt::algorithm>(i)) << "\n";
	}
}
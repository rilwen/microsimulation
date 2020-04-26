// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/log.hpp"

int main(int argc, char **argv) {
	averisera::Logging::set_level("DEBUG");
	 ::testing::InitGoogleTest(&argc, argv);
	 return RUN_ALL_TESTS();
}

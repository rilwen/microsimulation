/*
(C) Averisera Ltd 2017
*/
#include "microsim-simulator/fetus.hpp"
#include <gtest/gtest.h>
#include <sstream>

using namespace averisera;
using namespace averisera::microsim;

TEST(Fetus, Print) {
	const Fetus fetus(PersonAttributes(Sex::MALE, 0), Date(2001, 10, 10));
	std::stringstream ss;
	ss << fetus;
	ASSERT_EQ("((MALE, 0), 2001-10-10)", ss.str());
}

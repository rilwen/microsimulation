/*
 * (C) Averisera Ltd 2015
 */
#include "microsim-core/schedule_definition.hpp"
#include <gtest/gtest.h>

using namespace averisera;
using namespace averisera::microsim;

TEST(ScheduleDefinition, Test) {
    ScheduleDefinition def;
    def.start = Date(2015, 5, 1);
    def.end = Date(2016, 5, 1);
	def.frequency.type = PeriodType::MONTHS;
	def.frequency.size = 3;
}

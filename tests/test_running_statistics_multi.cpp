// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/running_statistics_multi.hpp"

using namespace averisera;

TEST(RunningStatisticsMulti, LowDim) {
    RunningStatisticsMulti<double> rs0(0);
    RunningStatisticsMulti<double> rs1(1);
    rs1.add(std::vector<double>({0.2}));
}

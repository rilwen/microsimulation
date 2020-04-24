/*
(C) Averisera Ltd 2014
*/
#include "test_jagged_2d_array.hpp"

typedef ::testing::Types<int, unsigned int, double> MyTypes;
INSTANTIATE_TYPED_TEST_CASE_P(My, Jagged2DArrayTest, MyTypes);

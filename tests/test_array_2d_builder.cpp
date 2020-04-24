/*
(C) Averisera Ltd 2014
*/
#include <gtest/gtest.h>
#include "core/array_2d_builder.hpp"
#include <string>

using namespace averisera;

TEST(Array2DBuilder, Test) {
	Array2DBuilder<std::string> builder;
	const Array2D<std::string> foo_bar(builder.add("Foo").new_row().add("Bar").arr());
	builder.wipe();
	builder.add("Ala").add("ma").add("kota").add(foo_bar);
	builder.new_row();
	builder.add("Lazy Fox");
	const Array2D<std::string> result(builder.arr());
	ASSERT_EQ(3u, result.size());
	ASSERT_EQ(std::vector<std::string>({ "Ala", "ma", "kota", "Foo" }), result[0]);
	ASSERT_EQ(std::vector<std::string>({ "", "", "", "Bar" }), result[1]);
	ASSERT_EQ(std::vector<std::string>({ "Lazy Fox" }), result[2]);
}

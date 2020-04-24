/*
* (C) Averisera Ltd 2015
*/
#include <gtest/gtest.h>
#include "microsim-simulator/history_factory.hpp"
#include "microsim-simulator/history.hpp"
#include "microsim-simulator/history/history_time_series.hpp"
#include "microsim-simulator/history/history_sparse.hpp"
#include <memory>

using namespace averisera::microsim;
using namespace averisera;

template <class T> void assert_ts_backed(const std::unique_ptr<History>& history) {
	ASSERT_NE(nullptr, dynamic_cast<HistoryTimeSeries<T>*>(history.get()));
}

template <class T> void test_build_backed_by_ts() {
	assert_ts_backed<T>(HistoryFactory::DENSE<T>()(""));
}

TEST(HistoryFactory, BuildBackedByTimeSeries) {
	test_build_backed_by_ts<double>();
	test_build_backed_by_ts<float>();
	test_build_backed_by_ts<int8_t>();
	test_build_backed_by_ts<int16_t>();
	test_build_backed_by_ts<int32_t>();
	test_build_backed_by_ts<uint8_t>();
	test_build_backed_by_ts<uint16_t>();
	test_build_backed_by_ts<uint32_t>();
	const auto history_ptr = HistoryFactory::DENSE<double>()("NAME");
	ASSERT_EQ("NAME", history_ptr->name());
}

template <class T> void assert_sparse(std::unique_ptr<History> history) {
	ASSERT_NE(nullptr, dynamic_cast<HistorySparse*>(history.get()));
}

template <class T> void test_build_sparse() {
	assert_sparse<T>(HistoryFactory::SPARSE<T>()(""));
}

TEST(HistoryFactory, BuildSparseBackedByTimeSeries) {
	test_build_sparse<double>();
	test_build_sparse<float>();
	test_build_sparse<int8_t>();
	test_build_sparse<int16_t>();
	test_build_sparse<int32_t>();
	test_build_sparse<uint8_t>();
	test_build_sparse<uint16_t>();
	test_build_sparse<uint32_t>();
    const auto history_ptr = HistoryFactory::SPARSE<int8_t>()("NOME");
	ASSERT_EQ("NOME", history_ptr->name());
}

TEST(HistoryFactory, DenseFromString) {
    ASSERT_EQ(std::make_pair(HistoryFactory::DENSE<double>(), std::string("double")), HistoryFactory::from_string("double"));
	ASSERT_EQ(std::make_pair(HistoryFactory::DENSE<float>(), std::string("float")), HistoryFactory::from_string("float"));
    ASSERT_EQ(std::make_pair(HistoryFactory::DENSE<int32_t>(), std::string("int32")), HistoryFactory::from_string("int32"));
    ASSERT_EQ(std::make_pair(HistoryFactory::DENSE<int8_t>(), std::string("int8")), HistoryFactory::from_string("int8"));
	ASSERT_EQ(std::make_pair(HistoryFactory::DENSE<int16_t>(), std::string("int16")), HistoryFactory::from_string("int16"));
    ASSERT_EQ(std::make_pair(HistoryFactory::DENSE<uint8_t>(), std::string("uint8")), HistoryFactory::from_string("uint8"));
	ASSERT_EQ(std::make_pair(HistoryFactory::DENSE<uint32_t>(), std::string("uint32")), HistoryFactory::from_string("uint32"));
	ASSERT_EQ(std::make_pair(HistoryFactory::DENSE<uint16_t>(), std::string("uint16")), HistoryFactory::from_string("uint16"));
}

TEST(HistoryFactory, SparseFromString) {
    ASSERT_EQ(std::make_pair(HistoryFactory::SPARSE<double>(), std::string("double")), HistoryFactory::from_string("sparse double"));
	ASSERT_EQ(std::make_pair(HistoryFactory::SPARSE<float>(), std::string("float")), HistoryFactory::from_string("sparse float"));
    ASSERT_EQ(std::make_pair(HistoryFactory::SPARSE<int32_t>(), std::string("int32")), HistoryFactory::from_string("sparse int32"));
	ASSERT_EQ(std::make_pair(HistoryFactory::SPARSE<int16_t>(), std::string("int16")), HistoryFactory::from_string("sparse int16"));
    ASSERT_EQ(std::make_pair(HistoryFactory::SPARSE<int8_t>(), std::string("int8")), HistoryFactory::from_string("sparse int8"));
    ASSERT_EQ(std::make_pair(HistoryFactory::SPARSE<uint8_t>(), std::string("uint8")), HistoryFactory::from_string("sparse uint8"));
	ASSERT_EQ(std::make_pair(HistoryFactory::SPARSE<uint16_t>(), std::string("uint16")), HistoryFactory::from_string("sparse uint16"));
	ASSERT_EQ(std::make_pair(HistoryFactory::SPARSE<uint32_t>(), std::string("uint32")), HistoryFactory::from_string("sparse uint32"));
}

TEST(HistoryFactory, AppendDouble) {
    auto history = HistoryFactory::DENSE<double>()("name");
    HistoryFactory::append(*history, "D[2011-Mar-01,0.2|2012-May-05,-1.31]");
    ASSERT_EQ(2u, history->size());
    ASSERT_EQ(0.2, history->as_double(0));
    ASSERT_EQ(Date(2011,3,1), history->date(0));
    ASSERT_EQ(-1.31, history->as_double(1));
    ASSERT_EQ(Date(2012,5,5), history->date(1));
}


TEST(HistoryFactory, AppendInt) {
    auto history = HistoryFactory::DENSE<int32_t>()("var");
    HistoryFactory::append(*history, "I[2011-Mar-01,2|2012-May-05,-1000]");
    ASSERT_EQ(2u, history->size());
    ASSERT_EQ(2, history->as_int(0));
    ASSERT_EQ(Date(2011,3,1), history->date(0));
    ASSERT_EQ(-1000, history->as_int(1));
    ASSERT_EQ(Date(2012,5,5), history->date(1));
}

TEST(HistoryFactory, FromStrings) {
	auto history = HistoryFactory::from_strings("int32", "I[2011-Mar-01,2|2012-May-05,-1000]", "imie");
	ASSERT_EQ(2u, history->size());
	ASSERT_EQ(2, history->as_int(0));
	ASSERT_EQ(Date(2011, 3, 1), history->date(0));
	ASSERT_EQ(-1000, history->as_int(1));
	ASSERT_EQ(Date(2012, 5, 5), history->date(1));
	ASSERT_EQ("imie", history->name());
	assert_ts_backed<int32_t>(history);

	history = HistoryFactory::from_strings("int16", "I[2011-Mar-01,2|2012-May-05,-1000]", "imie");
	assert_ts_backed<int16_t>(history);
	ASSERT_EQ(2u, history->size());

	history = HistoryFactory::from_strings("int8", "I[2011-Mar-01,2|2012-May-05,-100]", "imie");
	assert_ts_backed<int8_t>(history);
	ASSERT_EQ(2u, history->size());

	history = HistoryFactory::from_strings("uint32", "I[2011-Mar-01,2|2012-May-05,100000]", "imie");
	assert_ts_backed<uint32_t>(history);
	ASSERT_EQ(2u, history->size());

	history = HistoryFactory::from_strings("uint16", "I[2011-Mar-01,2|2012-May-05,1000]", "imie");
	assert_ts_backed<uint16_t>(history);
	ASSERT_EQ(2u, history->size());

	history = HistoryFactory::from_strings("uint8", "I[2011-Mar-01,2|2012-May-05,100]", "imie");
	assert_ts_backed<uint8_t>(history);
	ASSERT_EQ(2u, history->size());

	history = HistoryFactory::from_strings("double", "D[2011-Mar-01,1.2E10|2012-May-05,-0.1]", "imie");
	assert_ts_backed<double>(history);
	ASSERT_EQ(2u, history->size());

	history = HistoryFactory::from_strings("float", "D[2011-Mar-01,1000.2|2012-May-05,-0.1]", "imie");
	assert_ts_backed<float>(history);
	ASSERT_EQ(2u, history->size());
}

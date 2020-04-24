#include <gtest/gtest.h>
#include "core/bootstrap.hpp"
#include "core/running_statistics.hpp"

using namespace averisera;

TEST(Bootstrap, OneSided) {
	ASSERT_EQ(95u, Bootstrap<>::one_sided(100, 0.95));
}

TEST(Bootstrap, TwoSided) {
	const auto actual = Bootstrap<>::two_sided(100, 0.9);
	ASSERT_EQ(5u, actual.first);
	ASSERT_EQ(95u, actual.second);
}

TEST(Bootstrap, ResampleWithReplacement) {
	const size_t rows = 1;
	const size_t cols_in = 10;
	Eigen::MatrixXd in(rows, cols_in);
	const size_t cols_out = 1000;
	Eigen::MatrixXd out(rows, cols_out);
	for (size_t i = 0; i < cols_in; ++i) {
		in(0, i) = static_cast<double>(i % 2);
	}
	Bootstrap<> bootstrap;
	bootstrap.resample_with_replacement(in, out);
	RunningStatistics<double> rs;
	for (size_t i = 0; i < cols_out; ++i) {
		rs.add(out(0, i));
	}
	ASSERT_NEAR(0.5, rs.mean(), 0.02);

	std::vector<char> v_in = { 'a', 'b', 'a', 'c' };
	std::vector<char> v_out(v_in.size());
	std::vector<char> v_out2(v_in.size());
	bootstrap.resample_with_replacement(v_in, v_out);
	bootstrap.resample_with_replacement(v_in, v_out2.begin(), v_out2.end());
	for (auto it = v_out.begin(); it != v_out.end(); ++it) {
		ASSERT_TRUE('a' <= *it && *it <= 'c');
	}
	for (auto it = v_out2.begin(); it != v_out2.end(); ++it) {
		ASSERT_TRUE('a' <= *it && *it <= 'c');
	}
}

TEST(Bootstrap, ResampleDistribution) {
	Eigen::VectorXd in(3);
	in << 0.4, 0.3, 0.3;
	const size_t n = 1000;
	Eigen::MatrixXd out(3, n);
	Bootstrap<> bootstrap;
	for (size_t i = 0; i < n; ++i) {
        auto col = out.col(i);
		bootstrap.resample_distribution(in, 1000, col);
		ASSERT_NEAR(1.0, out.col(i).sum(), 1E-14) << i;
	}
	for (size_t i = 0; i < 3; ++i) {
		ASSERT_NEAR(in[i], out.row(i).mean(), 5E-3) << i;
	}

    auto col0 = out.col(0);
	bootstrap.resample_distribution(in, 0, col0);
	for (size_t i = 0; i < 3; ++i) {
		ASSERT_EQ(in[i], out(i, 0)) << i;
	}
}

TEST(Bootstrap, ResampleWithoutReplacement) {
	const size_t n = 1000;
	std::vector<int> in(n);
	std::iota(in.begin(), in.end(), 1);
	std::vector<int> out(n);
	Bootstrap<> bootstrap;
	bootstrap.resample_without_replacement(in, out);
	std::sort(out.begin(), out.end());
	std::sort(in.begin(), in.end());
	for (size_t i = 0; i < n; ++i) {
		ASSERT_EQ(i + 1, in[i]) << i;
		ASSERT_EQ(i + 1, out[i]) << i;
	}
	std::fill(out.begin(), out.end(), -1);
	bootstrap.resample_without_replacement(in, out.begin(), out.end());
	std::sort(out.begin(), out.end());
	std::sort(in.begin(), in.end());
	for (size_t i = 0; i < n; ++i) {
		ASSERT_EQ(i + 1, in[i]) << i;
		ASSERT_EQ(i + 1, out[i]) << i;
	}
	out.resize(4);
	std::fill(out.begin(), out.end(), -1); 
	bootstrap.resample_without_replacement(in, out);
	std::sort(out.begin(), out.end());
	for (size_t i = 0; i < 4; ++i) {
		ASSERT_GE(out[i], 1) << i;
		ASSERT_LE(out[i], static_cast<int>(n)) << i;
		if (i > 0) {
			ASSERT_GT(out[i], out[i - 1]) << i;
		}
	}
}

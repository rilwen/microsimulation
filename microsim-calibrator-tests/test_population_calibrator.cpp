// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-calibrator/population_calibrator.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(PopulationCalibrator, SyncEthnicGroupRanges) {
	std::vector<PopulationCalibrator::age_group_type> index({
		PopulationCalibrator::age_group_type(0, 5),
		PopulationCalibrator::age_group_type(5, 10)
	});
	Eigen::MatrixXd v0(2, 2);
	v0 << 0.9, 0.1,
		2, 0.2;
	Eigen::MatrixXd v1(2, 3);
	v1 << 0.45, 0.45, 0.1,
		1., 0.8, 0.2;
	Eigen::MatrixXd v2(2, 2);
	v2 << 0.5, 0.6,
		1., 1.1;
	std::vector<PopulationCalibrator::pop_data_type> dfs(3);
	const unsigned char a = 'A';
	const unsigned char b = 'B';
	const unsigned char c = 'C';
	dfs[0] = PopulationCalibrator::pop_data_type(v0, std::vector<Ethnicity::index_set_type>({
		Ethnicity::index_set_type({a, b}),
		Ethnicity::index_set_type({c, c})
	}), index);
	const std::vector<Ethnicity::index_set_type> cols({
		Ethnicity::index_set_type({a, a}),
		Ethnicity::index_set_type({b, b}),
		Ethnicity::index_set_type({c, c})
	});
	dfs[1] = PopulationCalibrator::pop_data_type(v1, cols, index);
	dfs[2] = PopulationCalibrator::pop_data_type(v2, std::vector<Ethnicity::index_set_type>({
		Ethnicity::index_set_type({a, a}),
		Ethnicity::index_set_type({b, c})
	}), index);
	const std::vector<PopulationCalibrator::pop_data_type> dfs_orig(dfs);
	PopulationCalibrator::sync_ethnic_group_ranges(dfs);
	ASSERT_EQ(3, dfs.size());
	ASSERT_EQ(dfs_orig[1], dfs[1]);
	for (const auto& df : dfs) {
		ASSERT_EQ(cols, df.columns()) << df;
		ASSERT_EQ(index, df.index()) << df;
	}
	for (size_t i = 0; i < dfs.size(); ++i) {
		for (size_t r = 0; r < index.size(); ++r) {
			ASSERT_NEAR(dfs_orig[i].row_values_ix(r).sum(), dfs[i].row_values_ix(r).sum(), 1E-14) << i << " " << r;
		}
		//std::cout << dfs[i] << std::endl;
	}
	for (size_t r = 0; r < index.size(); ++r) {
		ASSERT_NEAR(dfs_orig[0].ix(r, 0), dfs[0].ix(r, 0) + dfs[0].ix(r, 1), 1E-14);
		ASSERT_NEAR(dfs[1].ix(r, 0) / dfs[1].ix(r, 1), dfs[0].ix(r, 0) / dfs[0].ix(r, 1), 1E-14);
		ASSERT_NEAR(dfs_orig[2].ix(r, 1), dfs[2].ix(r, 1) + dfs[2].ix(r, 2), 1E-14);
		ASSERT_NEAR(dfs[1].ix(r, 2) / dfs[1].ix(r, 1), dfs[2].ix(r, 2) / dfs[2].ix(r, 1), 1E-14);
	}
}

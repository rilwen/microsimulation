// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include <random>
#include <cassert>
#include "core/crossvalidation.hpp"
#include "core/running_statistics.hpp"

struct ModelCrossSectional {
	double operator()(const averisera::ObservedDiscreteData& data, const std::vector<double>& extrap_years, Eigen::MatrixXd& extrap_probs) {
		assert(1 == extrap_years.size());
		ey.push_back(extrap_years[0]);
		const Eigen::VectorXd& ns = data.nbr_surveys;
		assert(1 == extrap_probs.cols());
		assert(1 == extrap_probs.rows());
		double a = 0;
		double b = 0;
		const size_t nlen = ns.size();
		assert(nlen < 3);
		for (size_t i = 0; i < nlen; ++i) {
			a += ns[i] * data.probs(0, i);
			b += ns[i];
		}
		extrap_probs(0, 0) = a / b;
		return 0.;
	}

	const char* name() const {
		return "ModelCrossSectional";
	}

	std::vector<double> ey;
};

class ModelLongitudinal {
public:
	ModelLongitudinal(unsigned int dim, bool for_kfold)
		: _dim(dim), _for_kfold(for_kfold) {}
	std::pair<double, double> operator()(const averisera::ObservedDiscreteData& calibration_data, const averisera::ObservedDiscreteData& test_data) {
		if (_for_kfold) {
			assert(2 == calibration_data.ltimes.size());
			assert(1 == test_data.ltimes.size());
		} else {
			assert(3 == calibration_data.ltimes.size());
			assert(3 == test_data.ltimes.size());
		}
		std::vector<double> probs(_dim);
		{
			std::vector<averisera::RunningStatistics<double>> stats(_dim);
			for (auto rit = calibration_data.ltrajs.row_begin(); rit != calibration_data.ltrajs.row_end(); ++rit) {
				for (auto it = (*rit).begin(); it != (*rit).end(); ++it) {
					const auto val = *it;
					for (unsigned int i = 0; i < _dim; ++i) {
						stats[i].add(i == val ? 1.0 : 0.0);
					}
				}
			}
			std::transform(stats.begin(), stats.end(), probs.begin(), [](const averisera::RunningStatistics<double>& st){return st.mean(); });
		}
		double test_ll = 0;
		for (auto rit = test_data.ltrajs.row_begin(); rit != test_data.ltrajs.row_end(); ++rit) {
			for (auto it = (*rit).begin(); it != (*rit).end(); ++it) {
				const double p = probs[*it];
				test_ll += log(p);
			}
		}
		if (_for_kfold) {
			return std::make_pair(std::numeric_limits<double>::quiet_NaN(), -test_ll);
		} else {
			double calib_ll = 0;
			for (auto rit = calibration_data.ltrajs.row_begin(); rit != calibration_data.ltrajs.row_end(); ++rit) {
				for (auto it = (*rit).begin(); it != (*rit).end(); ++it) {
					const double p = probs[*it];
					calib_ll += log(p);
				}
			}
			return std::make_pair(-calib_ll, -test_ll);
		}
	}
private:
	unsigned int _dim;
	bool _for_kfold;
};

TEST(CrossValidation, LOOCV) {
	const unsigned int dim = 1;
	const unsigned int T = 3;
	averisera::ObservedDiscreteData data(dim, T);
	Eigen::MatrixXd& p = data.probs;
	Eigen::VectorXd& ns = data.nbr_surveys;
	p(0, 0) = 1.1;
	p(0, 1) = 0.9;
	p(0, 2) = 1.0;
	ns[0] = 100;
	ns[1] = 80;
	ns[2] = 200;
	data.times = { 1999, 2000, 2001 };
	ModelCrossSectional mloocv;
	auto err_norm = [](double ns, Eigen::MatrixXd::ConstColXpr P, Eigen::MatrixXd::ColXpr Q){return ns * pow((P - Q).norm(), 2); };
	const std::vector<double> result = averisera::CrossValidation::cross_validation(data, mloocv, err_norm, false);
	ASSERT_EQ(data.times, mloocv.ey);
	ASSERT_EQ(3u, result.size());
	ASSERT_NEAR(100 * pow(1.1 - (0.9 * 80 + 1 * 200) / 280, 2), result[0], 1E-15);
	ASSERT_NEAR(80 * pow(0.9 - (1.1 * 100 + 1 * 200) / 300, 2), result[1], 1E-15);
	ASSERT_NEAR(200 * pow(1.0 - (1.1 * 100 + 0.9 * 80) / 180, 2), result[2], 1E-15);
}

TEST(CrossValidation, LOOCVLeaveFirst) {
	const unsigned int dim = 1;
	const unsigned int T = 3;
	averisera::ObservedDiscreteData data(dim, T);
	Eigen::MatrixXd& p = data.probs;
	Eigen::VectorXd& ns = data.nbr_surveys;
	p(0, 0) = 1.1;
	p(0, 1) = 0.9;
	p(0, 2) = 1.0;
	ns[0] = 100;
	ns[1] = 80;
	ns[2] = 200;
	data.times = { 1999, 2000, 2001 };
	ModelCrossSectional mloocv;
	auto err_norm = [](double ns, Eigen::MatrixXd::ConstColXpr P, Eigen::MatrixXd::ColXpr Q){return ns * pow((P - Q).norm(), 2); };
	const std::vector<double> result = averisera::CrossValidation::cross_validation(data, mloocv, err_norm, true);
	const std::vector<double> expected_ey = { 2000., 2001. };
	ASSERT_EQ(expected_ey, mloocv.ey);
	ASSERT_EQ(2u, result.size());
	ASSERT_NEAR(80 * pow(0.9 - (1.1 * 100 + 1 * 200) / 300, 2), result[0], 1E-15);
	ASSERT_NEAR(200 * pow(1.0 - (1.1 * 100 + 0.9 * 80) / 180, 2), result[1], 1E-15);
}

TEST(CrossValidation, kFold) {
	const unsigned int dim = 1;
	const unsigned int T = 3;
	averisera::ObservedDiscreteData data(dim, T);
	Eigen::MatrixXd& p = data.probs;
	Eigen::VectorXd& ns = data.nbr_surveys;
	p(0, 0) = 1.1;
	p(0, 1) = 0.9;
	p(0, 2) = 1.0;
	ns[0] = 100;
	ns[1] = 80;
	ns[2] = 200;
	data.times = { 1999, 2000, 2001 };
	const unsigned int k = T;
	const std::vector<double> expected = {
		100 * pow(1.1 - (0.9 * 80 + 1 * 200) / 280, 2),
		80 * pow(0.9 - (1.1 * 100 + 1 * 200) / 300, 2),
		200 * pow(1.0 - (1.1 * 100 + 0.9 * 80) / 180, 2)
	};
	int d = 0;
	std::mt19937 urng;
	urng.seed(42);
	for (unsigned int n = 0; n < 10; ++n) {
		ModelCrossSectional mloocv;
		const auto resultpair = averisera::CrossValidation::cross_validation_kfold(data, k, mloocv, [](double ns, Eigen::MatrixXd::ConstColXpr P, Eigen::MatrixXd::ColXpr Q){return ns * pow((P - Q).norm(), 2); }, urng, false);
		const auto result = resultpair.first;
		const auto indices = resultpair.second;
		ASSERT_EQ(T, mloocv.ey.size());
		ASSERT_EQ(T, indices.size());
		for (unsigned int i = 0; i < T; ++i) {
			d += std::abs(static_cast<int>(i - indices[i]));
			ASSERT_EQ(data.times[indices[i]], mloocv.ey[i]) << n << " " << i;
			ASSERT_NEAR(expected[indices[i]], result[i], 1E-15) << n << " " << i;
		}
		ASSERT_EQ(T, result.size());
	}
	ASSERT_TRUE(d > 0) << "Random reshufflings occurred";
}

TEST(CrossValidation, kFoldLeaveFirst) {
	const unsigned int dim = 1;
	const unsigned int T = 3;
	averisera::ObservedDiscreteData data(dim, T);
	Eigen::MatrixXd& p = data.probs;
	Eigen::VectorXd& ns = data.nbr_surveys;
	p(0, 0) = 1.1;
	p(0, 1) = 0.9;
	p(0, 2) = 1.0;
	ns[0] = 100;
	ns[1] = 80;
	ns[2] = 200;
	data.times = { 1999, 2000, 2001 };
	const unsigned int k = T - 1;
	const std::vector<double> expected = {
		80 * pow(0.9 - (1.1 * 100 + 1 * 200) / 300, 2),
		200 * pow(1.0 - (1.1 * 100 + 0.9 * 80) / 180, 2)
	};
	int d = 0;
	std::mt19937 urng;
	urng.seed(42);
	for (unsigned int n = 0; n < 10; ++n) {
		ModelCrossSectional mloocv;
		const auto resultpair = averisera::CrossValidation::cross_validation_kfold(data, k, mloocv, [](double ns, Eigen::MatrixXd::ConstColXpr P, Eigen::MatrixXd::ColXpr Q){return ns * pow((P - Q).norm(), 2); }, urng, true);
		const auto result = resultpair.first;
		const auto indices = resultpair.second;
		ASSERT_EQ(T - 1, mloocv.ey.size());
		ASSERT_EQ(T - 1, indices.size());
		for (unsigned int i = 0; i < T - 1; ++i) {
			d += std::abs(static_cast<int>(i + 1 - indices[i]));
			ASSERT_EQ(data.times[indices[i]], mloocv.ey[i]) << n << " " << i;
			ASSERT_NEAR(expected[indices[i] - 1], result[i], 1E-15) << n << " " << i;
		}
		ASSERT_EQ(T - 1, result.size());
	}
	ASSERT_TRUE(d > 0) << "Random reshufflings occurred";
}

TEST(CrossValidation, TSeries) {
	const unsigned int dim = 1;
	const unsigned int T = 3;
	averisera::ObservedDiscreteData data(dim, T);
	Eigen::MatrixXd& p = data.probs;
	Eigen::VectorXd& ns = data.nbr_surveys;
	p(0, 0) = 1.1;
	p(0, 1) = 0.9;
	p(0, 2) = 1.0;
	ns[0] = 100;
	ns[1] = 80;
	ns[2] = 200;
	data.times = { 1999, 2000, 2001 };
	ModelCrossSectional mloocv;
	const double result = averisera::CrossValidation::cross_validation_tseries(data, mloocv, [](double ns, Eigen::MatrixXd::ConstColXpr P, Eigen::MatrixXd::ColXpr Q){return ns * pow((P - Q).norm(), 2); });
	ASSERT_EQ(1u, mloocv.ey.size());
	ASSERT_EQ(data.times[2], mloocv.ey[0]);
	ASSERT_NEAR(200 * pow(1.0 - (1.1 * 100 + 0.9 * 80) / 180, 2), result, 1E-15);
}

TEST(CrossValidation, kFoldLongitudinal) {
	const unsigned int dim = 2;
	const std::vector<double> trow = { 0, 1 };
	const std::vector<std::vector<double>> times(3, trow);
	typedef averisera::ObservedDiscreteData::lcidx_t lt;
	std::vector<std::vector<lt>> trajs(3);
	trajs[0] = { 0, 0 };
	trajs[1] = { 1, 1 };
	trajs[2] = { 0, 1 };
	averisera::ObservedDiscreteData data;
	data.ltimes = averisera::Jagged2DArray<double>(times);
	data.ltrajs = averisera::Jagged2DArray<lt>(trajs);
	ModelLongitudinal model(dim, true);
	std::mt19937 urng;
	urng.seed(42);
	const auto result = averisera::CrossValidation::cross_validation_kfold_longitudinal(data, 3, model, urng);
	const double sum_result = std::accumulate(result.begin(), result.end(), 0.);
	const double sum_expected = -(4 * log(0.25) + 2 * log(0.5));
	ASSERT_NEAR(sum_result, sum_expected, 1E-12);
}

TEST(CrossValidation, TimeSeriesLongitudinal) {
	const unsigned int dim = 2;
	const std::vector<double> trow = { 0, 1, 3 };
	const std::vector<std::vector<double>> times(3, trow);
	typedef averisera::ObservedDiscreteData::lcidx_t lt;
	std::vector<std::vector<lt>> trajs(3);
	trajs[0] = { 0, 0, 0 };
	trajs[1] = { 1, 1, 1 };
	trajs[2] = { 0, 1, 0 };
	averisera::ObservedDiscreteData data;
	data.ltimes = averisera::Jagged2DArray<double>(times);
	data.ltrajs = averisera::Jagged2DArray<lt>(trajs);
	ModelLongitudinal model(dim, false);
	std::mt19937 urng;
	urng.seed(42);
	const double result = averisera::CrossValidation::cross_validation_tseries_longitudinal(data, model);
	const double expected = -(3 * log(0.5));
	ASSERT_NEAR(result, expected, 1E-12);
}

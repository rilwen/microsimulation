#include <gtest/gtest.h>
#include "microsim-core/markov_model.hpp"
#include "core/dates.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(MarkovModel, ApplyRelativeRisks) {
    const Period p = Period(PeriodType::MONTHS, 3);
    const unsigned int dim = 4;
    Eigen::MatrixXd pi(dim, dim);
    pi << 0.1, 0.2, 0.2, 0.3,
        0.2, 0.25, 0.25, 0.3,
        0.3, 0.25, 0.25, 0.3,
        0.4, 0.3, 0.3, 0.1;
    Eigen::VectorXd init_state(dim);
    init_state << 1.0, 0.0, 0.0, 0.0;
    MarkovModel m(pi, std::vector<Period>(dim, p), init_state);
    const std::vector<double> rr = { 1.2, 0.8, std::numeric_limits<double>::quiet_NaN(), 1.0 };
    std::vector<double> actual(dim);
    m.apply_relative_risks(0, rr, actual);
    ASSERT_EQ(pi(2, 0), actual[2]);
    ASSERT_NEAR(0.1235294118, actual[0], 1E-10);
    ASSERT_NEAR(0.1647058824, actual[1], 1E-10);
    ASSERT_NEAR(0.4117647059, actual[3], 1E-10);
}

TEST(MarkovModel, SelectNextState) {
    const unsigned int dim = 4;
    Eigen::MatrixXd pi(dim, dim);
    pi << 0.1, 0.2, 0.2, 0.3,
        0.2, 0.25, 0.25, 0.3,
        0.3, 0.25, 0.25, 0.3,
        0.4, 0.3, 0.3, 0.1;
    Eigen::VectorXd init_state(dim);
    init_state << 1.0, 0.0, 0.0, 0.0;
    const std::vector<Period> periods = { Period(PeriodType::MONTHS, 3), Period(PeriodType::MONTHS, 1), Period(PeriodType::YEARS, 1), Period(PeriodType::DAYS, 2) };
    MarkovModel m(pi, periods, init_state);
    const std::vector<double> rr = { 1.2, 0.8, std::numeric_limits<double>::quiet_NaN(), 1.0 };
    unsigned int next = m.select_next_state(0, rr, 0.11);
    ASSERT_EQ(next, 0u);
    next = m.select_next_state(0, rr, 0.13);
    ASSERT_EQ(next, 1u);
    auto ns = m.select_next_state(std::make_pair(Date(2014, 1, 1), 0), rr, 0.99);
    ASSERT_EQ(std::make_pair(Date(2014, 4, 1), 3u), ns);
    ASSERT_EQ(0u, m.select_initial_state(rr, 0.2));
}


TEST(MarkovModel, SelectNextStateZeroTransitionPeriod) {
	const unsigned int dim = 4;
	Eigen::MatrixXd pi(dim, dim);
	pi << 0.1, 0.2, 0.2, 0.,
		0.2, 0.25, 0.25, 0.,
		0.3, 0.25, 0.25, 0.,
		0.4, 0.3, 0.3, 1.;
	Eigen::VectorXd init_state(dim);
	init_state << 1.0, 0.0, 0.0, 0.0;
	const std::vector<Period> periods = { Period(PeriodType::MONTHS, 3), Period(PeriodType::MONTHS, 1), Period(PeriodType::YEARS, 1), Period() };
	MarkovModel m(pi, periods, init_state);
	const std::vector<double> rr = { 1.2, 0.8, std::numeric_limits<double>::quiet_NaN(), 1.0 };
	unsigned int next = m.select_next_state(0, rr, 0.11);
	ASSERT_EQ(next, 0u);
	next = m.select_next_state(0, rr, 0.13);
	ASSERT_EQ(next, 1u);
	auto ns = m.select_next_state(std::make_pair(Date(2014, 1, 1), 0), rr, 0.99);
	ASSERT_EQ(std::make_pair(Date(2014, 4, 1), 3u), ns);
	ASSERT_EQ(0u, m.select_initial_state(rr, 0.2));
	ns = m.select_next_state(std::make_pair(Date(2014, 1, 1), 3), rr, 0.99);
	ASSERT_EQ(std::make_pair(Date(2014, 1, 1), 3u), ns);
}

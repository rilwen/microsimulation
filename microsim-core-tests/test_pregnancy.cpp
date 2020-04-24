#include <gtest/gtest.h>
#include "microsim-core/hazard_curve_factory.hpp"
#include "microsim-core/anchored_hazard_curve.hpp"
#include "microsim-core/markov_model.hpp"
#include "microsim-core/pregnancy.hpp"
#include <sstream>

using namespace averisera;
using namespace averisera::microsim;

TEST(Pregnancy, Builder) {
    const unsigned int nstages = static_cast<unsigned int>(Pregnancy::State::SIZE);
    const Eigen::MatrixXd pi = Eigen::MatrixXd::Identity(nstages, nstages);
    const std::vector<Period> periods(nstages, Period(PeriodType::MONTHS, 3));
    Pregnancy::Builder builder;
    Eigen::VectorXd init_state_distr(nstages);
    init_state_distr.setZero();
    init_state_distr[0] = 1.0;
    MarkovModel mm(pi, periods, init_state_distr);
    builder.add_markov_model(new MarkovModel(pi, periods, init_state_distr), 2);
    builder.add_markov_model(new MarkovModel(pi, periods, init_state_distr), 1);
    Pregnancy pregnancy = builder.build();
    ASSERT_EQ(2u, pregnancy.nbr_stage_models());
    ASSERT_EQ(2u, pregnancy.transition_count(0));
    ASSERT_EQ(mm, pregnancy.stage_model(0));
    ASSERT_EQ(2u, Pregnancy::TERMINATING_EVENTS.size());
    ASSERT_NE(Pregnancy::TERMINATING_EVENTS.end(), std::find(Pregnancy::TERMINATING_EVENTS.begin(), Pregnancy::TERMINATING_EVENTS.end(), Pregnancy::Event::BIRTH));
}

TEST(Pregnancy, Simple) {
	Pregnancy pregnancy;
	ASSERT_EQ(1u, pregnancy.nbr_stage_models());
	ASSERT_EQ(static_cast<size_t>(Pregnancy::Event::SIZE), pregnancy.stage_model(0).dim());
	ASSERT_EQ(Period::months(9), pregnancy.stage_model(0).transition_period(static_cast<size_t>(Pregnancy::Event::CONCEPTION)));
	ASSERT_EQ(Period(), pregnancy.stage_model(0).transition_period(static_cast<size_t>(Pregnancy::Event::BIRTH)));
	ASSERT_EQ(Period(), pregnancy.stage_model(0).transition_period(static_cast<size_t>(Pregnancy::Event::MISCARRIAGE)));
}

TEST(Pregnancy, ResultingState) {
	ASSERT_EQ(Pregnancy::State::PREGNANT, Pregnancy::resulting_state(Pregnancy::Event::CONCEPTION, true));
	ASSERT_EQ(Pregnancy::State::PREGNANT, Pregnancy::resulting_state(Pregnancy::Event::CONCEPTION, false));
	ASSERT_EQ(Pregnancy::State::NOT_PREGNANT, Pregnancy::resulting_state(Pregnancy::Event::BIRTH, true));
	ASSERT_EQ(Pregnancy::State::NOT_PREGNANT, Pregnancy::resulting_state(Pregnancy::Event::BIRTH, false));
	ASSERT_EQ(Pregnancy::State::NOT_PREGNANT, Pregnancy::resulting_state(Pregnancy::Event::MISCARRIAGE, true));
	ASSERT_EQ(Pregnancy::State::NOT_PREGNANT, Pregnancy::resulting_state(Pregnancy::Event::MISCARRIAGE, false));
	ASSERT_EQ(Pregnancy::State::NOT_PREGNANT, Pregnancy::resulting_state(Pregnancy::Event::SIZE, true));
	ASSERT_EQ(Pregnancy::State::SIZE, Pregnancy::resulting_state(Pregnancy::Event::SIZE, false));
}

TEST(Pregnancy, IsTerminating) {
	ASSERT_FALSE(Pregnancy::is_terminating(Pregnancy::Event::CONCEPTION));
	ASSERT_TRUE(Pregnancy::is_terminating(Pregnancy::Event::BIRTH));
	ASSERT_TRUE(Pregnancy::is_terminating(Pregnancy::Event::MISCARRIAGE));
	ASSERT_FALSE(Pregnancy::is_terminating(Pregnancy::Event::SIZE));
}

TEST(Pregnancy, ConceptionEventStringRep) {
	std::stringstream ss;
	ss << Pregnancy::Event::CONCEPTION;
	ASSERT_EQ("CONCEPTION", ss.str());
}

TEST(Pregnancy, MiscarriageEventStringRep) {
	std::stringstream ss;
	ss << Pregnancy::Event::MISCARRIAGE;
	ASSERT_EQ("MISCARRIAGE", ss.str());
}

TEST(Pregnancy, BirthEventStringRep) {
	std::stringstream ss;
	ss << Pregnancy::Event::BIRTH;
	ASSERT_EQ("BIRTH", ss.str());
}

TEST(Pregnancy, NoneEventStringRep) {
	std::stringstream ss;
	ss << Pregnancy::Event::SIZE;
	ASSERT_EQ("NONE", ss.str());
}
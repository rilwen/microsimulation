// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-core/sex.hpp"
#include "microsim-simulator/operator/operator_birth.hpp"
#include "microsim-simulator/predicate_factory.hpp"
#include "microsim-simulator/procreation.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(OperatorBirth, Test) {
	OperatorBirth ob(PredicateFactory::make_sex(Sex::FEMALE, true));
	ASSERT_EQ(1, ob.user_requirements().size());
	ASSERT_EQ(Procreation::PREGNANCY_EVENT(), ob.user_requirements()[0]);
}

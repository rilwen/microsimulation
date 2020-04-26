// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/operator.hpp"
#include "microsim-simulator/predicate.hpp"
#include "mock_operator.hpp"

using namespace averisera;
using namespace averisera::microsim;

class MockPredicate: public Predicate<double> {
public:
    bool select(const double&, const Contexts&) const {
	return true;
    }

    bool select_out_of_context(const double&) const {
        return true;
    }

    bool always_true() const {
        return true;
    }

    bool always_true_out_of_context() const {
        return true;
    }
    
    MockPredicate* clone() const override {
        return new MockPredicate();
    }
    std::shared_ptr<const Predicate<double> > sum(std::shared_ptr<const Predicate<double> >) const override {
        return nullptr;
    }

	void print(std::ostream& os) const override {
		os << "Mock";
	}
};

typedef MockOperator<MockPredicate, double> OurMockOperator;


TEST(Operator, Relation) {
    OurMockOperator o1(true, vf(), vf());
    ASSERT_EQ(0, o1.relation(o1));
    OurMockOperator o2(true, vf(), vf({"Foo"}));
    ASSERT_EQ(0, o2.relation(o1));
    ASSERT_EQ(0, o1.relation(o2));
    OurMockOperator o3(true, vf({"Foo"}), vf({"Bar"}));
    ASSERT_EQ(1, o2.relation(o3));
    ASSERT_EQ(0, o3.relation(o1));
    OurMockOperator o4(true, vf({"Bar"}), vf({"Foo"}));
    ASSERT_THROW(o3.relation(o4), std::runtime_error);
    OurMockOperator o5(false, vf(), vf());
    ASSERT_EQ(o5.relation(o1), 1);
    ASSERT_EQ(o1.relation(o5), -1);
}

TEST(Operator, Sort) {
    std::vector<std::shared_ptr<Operator<double>>> operators;
    operators.push_back(std::make_shared<OurMockOperator>(false, vf(), vf()));
    operators.push_back(std::make_shared<OurMockOperator>(true, vf(), vf()));
    operators.push_back(std::make_shared<OurMockOperator>(true, vf({ "Foo" }), vf({"Bar"})));
    operators.push_back(std::make_shared<OurMockOperator>(false, vf(), vf({"Foo"})));
    std::vector<std::shared_ptr<Operator<double>>> sorted(operators);
    FeatureProvider<Feature>::sort(sorted);
    ASSERT_EQ(&*sorted[0], &*operators[1]);
    ASSERT_EQ(&*sorted[1], &*operators[2]);
    ASSERT_EQ(&*sorted[2], &*operators[0]);
    ASSERT_EQ(&*sorted[3], &*operators[3]);
}

TEST(Operator, AreAllRequirementsSatisfied) {
    std::vector<std::shared_ptr<Operator<double>>> operators;
    operators.push_back(std::make_shared<OurMockOperator>(false, vf(), vf()));
	ASSERT_FALSE(FeatureProvider<Feature>::are_all_requirements_satisfied(operators, {}, {}));
    operators.push_back(std::make_shared<OurMockOperator>(true, vf(), vf()));
	ASSERT_TRUE(FeatureProvider<Feature>::are_all_requirements_satisfied(operators, {}, {}));
	ASSERT_FALSE(FeatureProvider<Feature>::are_all_requirements_satisfied(operators, {}, {"FFF"}));
    operators.push_back(std::make_shared<OurMockOperator>(true, vf({ "Foo" }), vf({"Bar"})));
	ASSERT_FALSE(FeatureProvider<Feature>::are_all_requirements_satisfied(operators, {}, {}));
	ASSERT_TRUE(FeatureProvider<Feature>::are_all_requirements_satisfied(operators, { "Bar" }, {}));
    operators.push_back(std::make_shared<OurMockOperator>(false, vf({"Bar"}), vf()));
	ASSERT_TRUE(FeatureProvider<Feature>::are_all_requirements_satisfied(operators, {}, {}));
}

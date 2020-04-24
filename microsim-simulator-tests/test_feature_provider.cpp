/*
(C) Averisera Ltd 2015
*/
#include "microsim-simulator/feature_provider.hpp"
#include "microsim-simulator/operator_factory.hpp"
#include "microsim-core/anchored_hazard_curve.hpp"
#include "microsim-core/conception.hpp"
#include "microsim-core/hazard_curve.hpp"
#include "microsim-core/hazard_curve_factory.hpp"
#include "core/daycount.hpp"
#include "core/stl_utils.hpp"
#include <gtest/gtest.h>
#include <string>
#include <set>

using namespace averisera;
using namespace averisera::microsim;

typedef std::unordered_set<std::string> vf;

class MockFeatureProvider: public FeatureProvider<std::string> {
public:
    MockFeatureProvider(const vf& provided, const vf& required)
    : _req(required), _pro(provided) {}
    
    const vf& requires() const override {
        return _req;
    }
    
    const vf& provides() const override {
        return _pro;
    }
    
    bool operator==(const MockFeatureProvider& other) const {
        return _req == other._req && _pro == other._pro;
    }
private:
    vf _req;
    vf _pro;
};

std::ostream& operator<<(std::ostream& stream, const MockFeatureProvider& prov) {
    stream << "REQ=" << prov.requires() << ", PROV=" << prov.provides();
    return stream;
}

TEST(FeatureProvider, Relation) {
    const vf empty;
    MockFeatureProvider o1(empty, empty);
    ASSERT_EQ(0, o1.relation(o1));
    MockFeatureProvider o2(vf(), vf({"Foo"}));
    ASSERT_EQ(0, o2.relation(o1));
    ASSERT_EQ(0, o1.relation(o2));
    MockFeatureProvider o3(vf({"Foo"}), vf({"Bar"}));
    ASSERT_EQ(1, o2.relation(o3));
    ASSERT_EQ(0, o3.relation(o1));
    MockFeatureProvider o4(vf({"Bar"}), vf({"Foo"}));
    ASSERT_THROW(o3.relation(o4), std::runtime_error);
}

TEST(FeatureProvider, Sort) {
    const vf empty;
    std::vector<std::shared_ptr<MockFeatureProvider>> operators;
    operators.push_back(std::make_shared<MockFeatureProvider>(vf({ "Foo" }), vf({"Bar"})));
    operators.push_back(std::make_shared<MockFeatureProvider>(empty, empty));
    operators.push_back(std::make_shared<MockFeatureProvider>(vf(), vf({"Foo"})));
    operators.push_back(std::make_shared<MockFeatureProvider>(vf({"Bar"}), empty));
    std::vector<std::shared_ptr<MockFeatureProvider>> sorted(operators);
    FeatureProvider<std::string>::sort(sorted);
    ASSERT_EQ(*operators[3], *sorted[0]);
    ASSERT_EQ(*operators[0], *sorted[1]);
    ASSERT_EQ(*operators[1], *sorted[2]);
    ASSERT_EQ(*operators[2], *sorted[3]);
}

TEST(FeatureProvider, AreAllRequirementsSatisfied) {
    const vf empty;
    std::vector<std::shared_ptr<FeatureProvider<std::string>>> operators;
    operators.push_back(std::make_shared<MockFeatureProvider>(empty, vf()));
	ASSERT_TRUE(FeatureProvider<std::string>::are_all_requirements_satisfied(operators, {}, {}));
	ASSERT_FALSE(FeatureProvider<std::string>::are_all_requirements_satisfied(operators, {}, {"Foo"}));
	ASSERT_TRUE(FeatureProvider<std::string>::are_all_requirements_satisfied(operators, {"Foo"}, { "Foo" }));
    operators.push_back(std::make_shared<MockFeatureProvider>(vf({ "Foo" }), vf({"Bar"})));
	ASSERT_FALSE(FeatureProvider<std::string>::are_all_requirements_satisfied(operators, {}, {}));
	ASSERT_TRUE(FeatureProvider<std::string>::are_all_requirements_satisfied(operators, { "Bar" }, {}));
    operators.push_back(std::make_shared<MockFeatureProvider>(vf({"Bar"}), vf()));
	ASSERT_TRUE(FeatureProvider<std::string>::are_all_requirements_satisfied(operators, {}, {}));
}

TEST(FeatureProvider, SortOperators) {
	const Date start_date(1990, 1, 1);
	static const uint8_t ETHN_BLACK = 0;
	static const uint8_t ETHN_WHITE = 1;
	const unsigned int min_childbearing_age = 15;
	const unsigned int max_childbearing_age = 45;
	const unsigned int zero_fertility_period_months = 3;
	const Period zero_fertility_period(PeriodType::MONTHS, zero_fertility_period_months);

	std::vector<std::shared_ptr<Operator<Person>>> person_operators;

	person_operators.push_back(OperatorFactory::make_birth(min_childbearing_age, max_childbearing_age));
	person_operators.push_back(OperatorFactory::make_fetus_generator_simple(min_childbearing_age, max_childbearing_age, 0.52)); // slightly more girls than boys
	person_operators.push_back(OperatorFactory::make_pregnancy(Pregnancy(), nullptr, min_childbearing_age, max_childbearing_age));
	const double conc_hazard_black = 0.1;
	const double conc_hazard_white = 0.05;
	const auto daycount = Daycount::DAYS_365();
	person_operators.push_back(OperatorFactory::make_conception(Conception(AnchoredHazardCurve::build(start_date, daycount, HazardCurveFactory::make_flat(conc_hazard_black))),
		std::vector<std::shared_ptr<const RelativeRisk<Person>>>(),
		PredicateFactory::make_ethnicity(Ethnicity::index_set_type({ ETHN_BLACK }), true),
		nullptr, nullptr, min_childbearing_age, max_childbearing_age, zero_fertility_period));
	person_operators.push_back(OperatorFactory::make_conception(Conception(AnchoredHazardCurve::build(start_date, daycount, HazardCurveFactory::make_flat(conc_hazard_white))),
		std::vector<std::shared_ptr<const RelativeRisk<Person>>>(),
		PredicateFactory::make_ethnicity(Ethnicity::index_set_type({ ETHN_WHITE }), true),
		nullptr, nullptr, min_childbearing_age, max_childbearing_age, zero_fertility_period));

	// sort the operators
	FeatureProvider<Feature>::sort(person_operators);

	// check order:
    for (size_t i = 0; i < person_operators.size(); ++i) {
        for (size_t j = 0; j < i; ++j) {
            const int rel = person_operators[j]->relation(*person_operators[i]);
            ASSERT_NE(1, rel) << person_operators[i]->name() << " " << person_operators[j]->name();
        }
    }
	std::vector<std::string> names(person_operators.size());
	std::transform(person_operators.begin(), person_operators.end(), names.begin(), [](std::shared_ptr<Operator<Person>> op_ptr) {
		return op_ptr->name();
	});
	const std::vector<std::string> expected_names({ "Conception", "Conception", "FetusGeneratorSimple", "Pregnancy", "Birth" });
	ASSERT_EQ(expected_names, names);
}

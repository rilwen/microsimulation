#include <gtest/gtest.h>
#include "microsim-simulator/contexts.hpp"
#include "microsim-simulator/dispatcher_factory.hpp"
#include "microsim-simulator/history_factory.hpp"
#include "microsim-simulator/immutable_context.hpp"
#include "microsim-simulator/mutable_context.hpp"
#include "microsim-simulator/predicate_factory.hpp"
#include "microsim-simulator/person.hpp"
#include "microsim-simulator/procreation.hpp"
#include "microsim-core/person_attributes.hpp"
#include <limits>

using namespace averisera;
using namespace averisera::microsim;

static const auto DISPATCHER = ImmutableContext::person_history_dispatcher_ptr_t(DispatcherFactory::make_constant<Person>(HistoryFactory::DENSE<double>(), PredicateFactory::make_true<Person>()));

template <class T> static std::string get_name(const Predicate<T>& pred) {
	std::stringstream ss;
	pred.print(ss);
	return ss.str();
}

TEST(PredicateFactory, PredTrue) {
    const auto pred = PredicateFactory::make_true<int>();
    ASSERT_TRUE(pred->always_true());
    ASSERT_TRUE(pred->always_true_out_of_context());
    Contexts ctx;
    ASSERT_TRUE(pred->select(2, ctx));
	ASSERT_TRUE(pred->select_alive(2, ctx));
    ASSERT_TRUE(pred->select_out_of_context(2));
	ASSERT_EQ("True", get_name(*pred));
}

TEST(PredicateFactory, PredNot) {
    auto nieprawda = std::shared_ptr<const Predicate<int>>(PredicateFactory::make_not(std::shared_ptr<const Predicate<int>>(PredicateFactory::make_true<int>())));
	ASSERT_EQ("Not(True)", get_name(*nieprawda));
    ASSERT_FALSE(nieprawda->always_true());
    ASSERT_FALSE(nieprawda->always_true_out_of_context());
    Contexts ctx;
    ASSERT_FALSE(nieprawda->select(2, ctx));
	ASSERT_FALSE(nieprawda->select_alive(2, ctx));
    ASSERT_FALSE(nieprawda->select_out_of_context(2));
    auto prawda = PredicateFactory::make_not(nieprawda);
    ASSERT_FALSE(prawda->always_true()); // make_not doesn't know that nieprawda is always false
    ASSERT_TRUE(prawda->select(2, ctx));
	ASSERT_TRUE(prawda->select_alive(2, ctx));
    ASSERT_FALSE(prawda->always_true_out_of_context()); // make_not doesn't know that nieprawda is always false
    ASSERT_TRUE(prawda->select_out_of_context(2));
    auto prawda2 = nieprawda->negate(); 
    ASSERT_TRUE(prawda2->always_true()); // .negate() knows that ~(~x) == x
    ASSERT_TRUE(prawda2->always_true_out_of_context());
    ASSERT_TRUE(prawda2->select(2, ctx));
	ASSERT_TRUE(prawda2->select_alive(2, ctx));
    ASSERT_TRUE(prawda2->select_out_of_context(2));
}

TEST(PredicateFactory, PredAnd) {
    Contexts ctx;
    auto pred = PredicateFactory::make_and<int>({PredicateFactory::make_true<int>(), PredicateFactory::make_true<int>()});
    ASSERT_TRUE(pred->select(2, ctx));
	ASSERT_TRUE(pred->select_alive(2, ctx));
    ASSERT_TRUE(pred->always_true());
    pred = PredicateFactory::make_and<int>({PredicateFactory::make_true<int>(), PredicateFactory::make_not<int>(PredicateFactory::make_true<int>())});
    ASSERT_FALSE(pred->select(2, ctx));
	ASSERT_FALSE(pred->select_alive(2, ctx));
    ASSERT_FALSE(pred->always_true());
}

TEST(PredicateFactory, PredOr) {
    Contexts ctx;
    auto pred = PredicateFactory::make_or<int>({PredicateFactory::make_true<int>(), PredicateFactory::make_true<int>()});
    ASSERT_TRUE(pred->select(2, ctx));
	ASSERT_TRUE(pred->select_alive(2, ctx));
    ASSERT_TRUE(pred->always_true());
    ASSERT_TRUE(pred->select_out_of_context(2));
    ASSERT_TRUE(pred->always_true_out_of_context());
    pred = PredicateFactory::make_or<int>({PredicateFactory::make_true<int>(), PredicateFactory::make_not<int>(PredicateFactory::make_true<int>())});
    ASSERT_TRUE(pred->select(2, ctx));
	ASSERT_TRUE(pred->select_alive(2, ctx));
    ASSERT_TRUE(pred->always_true());
    ASSERT_TRUE(pred->select_out_of_context(2));
    ASSERT_TRUE(pred->always_true_out_of_context());
}

TEST(PredicateFactory, PredSex) {
    auto pred = PredicateFactory::make_sex(Sex::FEMALE, false);
	ASSERT_EQ("Sex(FEMALE, 0)", get_name(*pred));
    ASSERT_FALSE(pred->always_true());
    ASSERT_FALSE(pred->always_true_out_of_context());
    const Person she(1, PersonAttributes(Sex::FEMALE, 1), Date(2018, 1, 1));
    const Person he(2, PersonAttributes(Sex::MALE, 1), Date(2018, 1, 1));
    Contexts ctx;
    ASSERT_FALSE(pred->select(he, ctx));
	ASSERT_FALSE(pred->select_alive(he, ctx));
    ASSERT_TRUE(pred->select(she, ctx));
	ASSERT_TRUE(pred->select_alive(she, ctx));
    ASSERT_FALSE(pred->select_out_of_context(he));
    ASSERT_TRUE(pred->select_out_of_context(she));
    pred = PredicateFactory::make_sex(Sex::FEMALE);
    ctx = Contexts(Date(2020, 1, 1));
    ASSERT_FALSE(pred->select(he, ctx));
    ASSERT_TRUE(pred->select(she, ctx));
	ASSERT_FALSE(pred->select_alive(he, ctx));
	ASSERT_TRUE(pred->select_alive(she, ctx));
    ASSERT_FALSE(pred->select_out_of_context(he));
    ASSERT_TRUE(pred->select_out_of_context(she));
    ctx = Contexts(Date(2010, 1, 1));
    ASSERT_FALSE(pred->select(he, ctx));
    ASSERT_FALSE(pred->select(she, ctx));	
}

TEST(PredicateFactory, PredEthnicity) {
    auto pred = PredicateFactory::make_ethnicity(2, 3, false);
	ASSERT_EQ("EthnicityRange([2, 3], 0)", get_name(*pred));
    ASSERT_FALSE(pred->always_true());
    ASSERT_FALSE(pred->always_true_out_of_context());
    const Person eth1(1, PersonAttributes(Sex::MALE, 1), Date(2018, 1, 1));
    const Person eth2(2, PersonAttributes(Sex::MALE, 2), Date(2018, 1, 1));
    Contexts ctx;
    ASSERT_FALSE(pred->select(eth1, ctx));
    ASSERT_TRUE(pred->select(eth2, ctx));
	ASSERT_FALSE(pred->select_alive(eth1, ctx));
	ASSERT_TRUE(pred->select_alive(eth2, ctx));
    ASSERT_FALSE(pred->select_out_of_context(eth1));
    ASSERT_TRUE(pred->select_out_of_context(eth2));
    pred = PredicateFactory::make_ethnicity(2, 3, true);
    ctx = Contexts(Date(2020, 1, 1));
    ASSERT_FALSE(pred->select(eth1, ctx));
    ASSERT_TRUE(pred->select(eth2, ctx));
	ASSERT_FALSE(pred->select_alive(eth1, ctx));
	ASSERT_TRUE(pred->select_alive(eth2, ctx));
    ctx = Contexts(Date(2010, 1, 1));
    ASSERT_FALSE(pred->select(eth1, ctx));
    ASSERT_FALSE(pred->select(eth2, ctx));

	pred = PredicateFactory::make_ethnicity(2, 2, false);
	ASSERT_EQ("EthnicitySingle([2], 0)", get_name(*pred));
	ASSERT_FALSE(pred->select(eth1, ctx));
	ASSERT_TRUE(pred->select(eth2, ctx));
	ASSERT_FALSE(pred->select_alive(eth1, ctx));
	ASSERT_TRUE(pred->select_alive(eth2, ctx));

	pred = PredicateFactory::make_ethnicity(std::unordered_set<PersonAttributes::ethnicity_t>({ 2, 5 }), false);
	ASSERT_EQ("EthnicitySet([2, 5], 0)", get_name(*pred));
	ASSERT_FALSE(pred->select(eth1, ctx));
	ASSERT_TRUE(pred->select(eth2, ctx));
	ASSERT_FALSE(pred->select_alive(eth1, ctx));
	ASSERT_TRUE(pred->select_alive(eth2, ctx));
}

TEST(PredicateFactory, PredAge) {
    auto pred = PredicateFactory::make_age(20, 25, false);
	ASSERT_EQ("Age(20, 25, 0)", get_name(*pred));
    ASSERT_FALSE(pred->always_true());
    ASSERT_TRUE(pred->always_true_out_of_context());
    Person young(1, PersonAttributes(Sex::MALE, 1), Date(1994, 1, 1));
    const Person old(2, PersonAttributes(Sex::MALE, 2), Date(1964, 1, 1));
    Contexts ctx(Date(2015, 9, 16));
    ASSERT_FALSE(pred->select(old, ctx));
    ASSERT_TRUE(pred->select(young, ctx));
    ASSERT_TRUE(pred->select_out_of_context(old));
    ASSERT_TRUE(pred->select_out_of_context(young));
	ASSERT_TRUE(pred->select_alive(young, ctx));
	pred = PredicateFactory::make_age(20, 25);
    ASSERT_FALSE(pred->select(old, ctx));
    ASSERT_TRUE(pred->select(young, ctx));
	ASSERT_FALSE(pred->select_alive(old, ctx));
	ASSERT_TRUE(pred->select_alive(young, ctx));
	ASSERT_TRUE(pred->select_out_of_context(old));
    ASSERT_TRUE(pred->select_out_of_context(young));
    ctx = Contexts(Date(2040, 1, 1));
    ASSERT_FALSE(pred->select(old, ctx));
    ASSERT_FALSE(pred->select(young, ctx));
	ASSERT_FALSE(pred->select_alive(old, ctx));
	ASSERT_FALSE(pred->select_alive(young, ctx));
	young.die(Date(2014, 5, 5));
    ctx = Contexts(Date(2015, 9, 16));
    ASSERT_FALSE(pred->select(old, ctx));
    ASSERT_FALSE(pred->select(young, ctx));
}


TEST(PredicateFactory, PredYearOfBirth) {
    auto pred = PredicateFactory::make_year_of_birth(2000, 2004, false);
	ASSERT_EQ("YearOfBirth(2000, 2004, 0)", get_name(*pred));
    ASSERT_FALSE(pred->always_true());
    ASSERT_FALSE(pred->always_true_out_of_context());
    Person p1(1, PersonAttributes(Sex::MALE, 0), Date(2006, 1, 1));
    Person p2(2, PersonAttributes(Sex::MALE, 0), Date(2001, 1, 1));
    Contexts ctx;
    ASSERT_FALSE(pred->select(p1, ctx));
    ASSERT_TRUE(pred->select(p2, ctx));
	ASSERT_FALSE(pred->select_alive(p1, ctx));
	ASSERT_TRUE(pred->select_alive(p2, ctx));
	ASSERT_FALSE(pred->select_out_of_context(p1));
    ASSERT_TRUE(pred->select_out_of_context(p2));
    pred = PredicateFactory::make_year_of_birth(2000, 2004);
    ctx = Contexts(Date(2020, 1, 1));
    ASSERT_FALSE(pred->select(p1, ctx));
    ASSERT_TRUE(pred->select(p2, ctx));
	ASSERT_FALSE(pred->select_alive(p1, ctx));
	ASSERT_TRUE(pred->select_alive(p2, ctx));
	ASSERT_FALSE(pred->select_out_of_context(p1));
    ASSERT_TRUE(pred->select_out_of_context(p2));
    p1.die(Date(2015, 1, 1));
    p2.die(Date(2015, 1, 1));
    ASSERT_FALSE(pred->select(p1, ctx));
    ASSERT_FALSE(pred->select(p2, ctx));
    ASSERT_FALSE(pred->select_out_of_context(p1));
    ASSERT_TRUE(pred->select_out_of_context(p2));
}

static std::vector<std::unique_ptr<History>> build_histories() {
    std::vector<std::unique_ptr<History>> hv;
    hv.push_back(HistoryFactory::DENSE<double>()(""));
    return hv;
}

TEST(PredicateFactory, PredVariableRangeDouble) {
    auto pred = PredicateFactory::make_variable_range<Person>("X", 0.1, 0.2);
	ASSERT_EQ("VariableRange(\"X\", 0.1, 0.2, 0)", get_name(*pred));
    auto pred2 = PredicateFactory::make_variable_range<Person>("X", 0.1, 0.2, true);
	ASSERT_EQ("VariableRange(\"X\", 0.1, 0.2, 1)", get_name(*pred2));
    ASSERT_FALSE(pred->always_true());
    ASSERT_TRUE(pred->always_true_out_of_context());
    Person p1(1, PersonAttributes(Sex::MALE, 0), Date(2001, 1, 1));
    Person p2(2, PersonAttributes(Sex::MALE, 0), Date(2001, 1, 1));
    p1.set_histories(build_histories());
    p2.set_histories(build_histories());
    const Date d1(2012, 1, 1);
    const Date d2(2012, 6, 1);
    auto imm_ctx = std::make_shared<ImmutableContext>(Schedule({ d1, d2 }), Ethnicity::IndexConversions());
    const auto idx = imm_ctx->register_person_variable("X", DISPATCHER);
    auto mut_ctx = std::make_shared<MutableContext>();
    Contexts ctx(imm_ctx, mut_ctx);
    ASSERT_FALSE(pred->select(p1, ctx));
    ASSERT_FALSE(pred->select(p2, ctx));
	ASSERT_FALSE(pred->select_alive(p1, ctx));
	ASSERT_FALSE(pred->select_alive(p2, ctx));
	ASSERT_TRUE(pred->select_out_of_context(p1));
    ASSERT_TRUE(pred->select_out_of_context(p2));
    ASSERT_TRUE(pred2->select(p1, ctx));
    ASSERT_TRUE(pred2->select(p2, ctx));
	ASSERT_TRUE(pred2->select_alive(p1, ctx));
	ASSERT_TRUE(pred2->select_alive(p2, ctx));
	ASSERT_TRUE(pred2->select_out_of_context(p1));
    ASSERT_TRUE(pred2->select_out_of_context(p2));
    p1.history(idx).append(d1, 0.0);
    p2.history(idx).append(d1, 0.15);
    ASSERT_FALSE(pred->select(p1, ctx));
    ASSERT_TRUE(pred->select(p2, ctx));
	ASSERT_TRUE(pred->select_alive(p2, ctx));
    ASSERT_TRUE(pred->select_out_of_context(p1));
    ASSERT_TRUE(pred->select_out_of_context(p2));
    ASSERT_FALSE(pred2->select(p1, ctx));
    ASSERT_TRUE(pred2->select(p2, ctx));    
	ASSERT_FALSE(pred2->select_alive(p1, ctx));
	ASSERT_TRUE(pred2->select_alive(p2, ctx));
    ASSERT_TRUE(pred2->select_out_of_context(p1));
    ASSERT_TRUE(pred2->select_out_of_context(p2));

    auto pred3 = PredicateFactory::make_variable_range<Person>("X", -std::numeric_limits<History::double_t>::infinity(), std::numeric_limits<History::double_t>::infinity(), true);
    ASSERT_TRUE(pred3->always_true());
}

TEST(PredicateFactory, PredVariableRangeInt) {
    auto pred = PredicateFactory::make_variable_range<Person, History::int_t>("X", 10, 20);
    auto pred2 = PredicateFactory::make_variable_range<Person, History::int_t>("X", 10, 20, true);
    ASSERT_FALSE(pred->always_true());
    ASSERT_TRUE(pred->always_true_out_of_context());
    Person p1(1, PersonAttributes(Sex::MALE, 0), Date(2001, 1, 1));
    Person p2(2, PersonAttributes(Sex::MALE, 0), Date(2001, 1, 1));
    p1.set_histories(build_histories());
    p2.set_histories(build_histories());
    const Date d1(2012, 1, 1);
    const Date d2(2012, 6, 1);
    auto imm_ctx = std::make_shared<ImmutableContext>(Schedule({ d1, d2 }));
    const auto idx = imm_ctx->register_person_variable("X", DISPATCHER);
    auto mut_ctx = std::make_shared<MutableContext>();
    Contexts ctx(imm_ctx, mut_ctx);
    ASSERT_FALSE(pred->select(p1, ctx));
    ASSERT_FALSE(pred->select(p2, ctx));
	ASSERT_FALSE(pred->select_alive(p1, ctx));
	ASSERT_FALSE(pred->select_alive(p2, ctx));
	ASSERT_TRUE(pred->select_out_of_context(p1));
    ASSERT_TRUE(pred->select_out_of_context(p2));
    ASSERT_TRUE(pred2->select(p1, ctx));
    ASSERT_TRUE(pred2->select(p2, ctx));
	ASSERT_TRUE(pred2->select_alive(p1, ctx));
	ASSERT_TRUE(pred2->select_alive(p2, ctx));
	ASSERT_TRUE(pred2->select_out_of_context(p1));
    ASSERT_TRUE(pred2->select_out_of_context(p2));
    p1.history(idx).append(d1, (long int)0);
    p2.history(idx).append(d1, (long int)15);
    ASSERT_FALSE(pred->select(p1, ctx));
    ASSERT_TRUE(pred->select(p2, ctx));
	ASSERT_FALSE(pred->select_alive(p1, ctx));
	ASSERT_TRUE(pred->select_alive(p2, ctx));
	ASSERT_TRUE(pred->select_out_of_context(p1));
    ASSERT_TRUE(pred->select_out_of_context(p2));
    ASSERT_FALSE(pred2->select(p1, ctx));
    ASSERT_TRUE(pred2->select(p2, ctx));    
	ASSERT_FALSE(pred2->select_alive(p1, ctx));
	ASSERT_TRUE(pred2->select_alive(p2, ctx));
	ASSERT_TRUE(pred2->select_out_of_context(p1));
    ASSERT_TRUE(pred2->select_out_of_context(p2));

    auto pred3 = PredicateFactory::make_variable_range<Person>("X", std::numeric_limits<History::int_t>::min(), std::numeric_limits<History::int_t>::max(), true);
    ASSERT_TRUE(pred3->always_true());
}

static std::vector<std::unique_ptr<History>> build_int_histories() {
    std::vector<std::unique_ptr<History>> hv;
    hv.push_back(HistoryFactory::DENSE<int>()("int"));
    return hv;
}

static const auto DISPATCHER_PREGNANCY = ImmutableContext::person_history_dispatcher_ptr_t(DispatcherFactory::make_constant<Person>(HistoryFactory::DENSE<int>(), PredicateFactory::make_sex(Sex::FEMALE)));

TEST(PredicateFactory, PredPregnancyAtStart) {
    std::shared_ptr<const Predicate<Person> > pred = PredicateFactory::make_pregnancy(Pregnancy::State::PREGNANT, false, true);
	ASSERT_EQ("Pregnancy(1, 0, 1)", get_name(*pred));
    std::shared_ptr<const Predicate<Person> > pred2 = PredicateFactory::make_pregnancy(Pregnancy::State::PREGNANT, true, true);
    std::shared_ptr<const Predicate<Person> > pred3 = PredicateFactory::make_pregnancy(Pregnancy::State::NOT_PREGNANT, false, true);
    std::shared_ptr<const Predicate<Person> > pred4 = PredicateFactory::make_pregnancy(Pregnancy::State::NOT_PREGNANT, true, true);
	ASSERT_FALSE(pred->always_true());
    ASSERT_FALSE(pred->always_true_out_of_context());
    ASSERT_FALSE(pred2->always_true());
    ASSERT_FALSE(pred2->always_true_out_of_context());
    ASSERT_FALSE(pred3->always_true());
    ASSERT_FALSE(pred3->always_true_out_of_context());
    ASSERT_FALSE(pred4->always_true());
    ASSERT_FALSE(pred4->always_true_out_of_context());

    Person p1(1, PersonAttributes(Sex::FEMALE, 0), Date(2001, 1, 1));
    Person p2(2, PersonAttributes(Sex::FEMALE, 0), Date(2015, 1, 1));
    Person p3(2, PersonAttributes(Sex::MALE, 0), Date(2001, 1, 1));
    p1.set_histories(build_int_histories());
    p2.set_histories(build_int_histories());
    p3.set_histories(build_int_histories());

    const auto preds = std::vector<std::shared_ptr<const Predicate<Person> >>{pred, pred2, pred3, pred4};
    for (auto pr: preds) {
        ASSERT_TRUE(pr->select_out_of_context(p1));
        ASSERT_TRUE(pr->select_out_of_context(p2));
        ASSERT_FALSE(pr->select_out_of_context(p3));
    }

    const Date d1(2012, 1, 1);
    const Date d2(2020, 6, 1);
    auto imm_ctx = std::make_shared<ImmutableContext>(Schedule({ d1, d2 }));
    const auto idx = imm_ctx->register_person_variable(Procreation::PREGNANCY_EVENT(), DISPATCHER_PREGNANCY);
    auto mut_ctx = std::make_shared<MutableContext>();
    Contexts ctx(imm_ctx, mut_ctx);

    for (auto pr: preds) {
        ASSERT_FALSE(pr->select(p3, ctx));
    }

    // Men are never selected (even if pregnant)
    p3.history(idx).append_int(d1, static_cast<int>(Pregnancy::Event::CONCEPTION));
	for (size_t i = 0; i < preds.size(); ++i) {
		const auto& pr = preds[i];
        ASSERT_FALSE(pr->select(p3, ctx)) << idx;
		ASSERT_FALSE(pr->select_alive(p3, ctx)) << idx;		
    }

    ASSERT_FALSE(pred->select(p1, ctx));
    ASSERT_FALSE(pred2->select(p1, ctx));
    ASSERT_TRUE(pred3->select(p1, ctx));
    ASSERT_TRUE(pred4->select(p1, ctx));

	ASSERT_FALSE(pred->select_alive(p1, ctx));
	ASSERT_FALSE(pred2->select_alive(p1, ctx));
	ASSERT_TRUE(pred3->select_alive(p1, ctx));
	ASSERT_TRUE(pred4->select_alive(p1, ctx));

    ASSERT_FALSE(pred->select(p2, ctx));
    ASSERT_FALSE(pred2->select(p2, ctx));
    ASSERT_TRUE(pred3->select(p2, ctx));
    ASSERT_FALSE(pred4->select(p2, ctx));

	ASSERT_FALSE(pred->select_alive(p2, ctx));
	ASSERT_FALSE(pred2->select_alive(p2, ctx));
	ASSERT_TRUE(pred3->select_alive(p2, ctx));
	ASSERT_TRUE(pred4->select_alive(p2, ctx)); // because we don't check is_alive

    p1.history(idx).append_int(d1, static_cast<int>(Pregnancy::Event::BIRTH));
    p1.history(idx).append_int(d2, static_cast<int>(Pregnancy::Event::CONCEPTION));

    ASSERT_FALSE(pred->select(p1, ctx));
    ASSERT_FALSE(pred2->select(p1, ctx));
    ASSERT_TRUE(pred3->select(p1, ctx));
    ASSERT_TRUE(pred4->select(p1, ctx));

	ASSERT_FALSE(pred->select_alive(p1, ctx));
	ASSERT_FALSE(pred2->select_alive(p1, ctx));
	ASSERT_TRUE(pred3->select_alive(p1, ctx));
	ASSERT_TRUE(pred4->select_alive(p1, ctx));

    ctx.mutable_ctx().advance_date_index();

    ASSERT_TRUE(pred->select(p1, ctx));
    ASSERT_TRUE(pred2->select(p1, ctx));
    ASSERT_FALSE(pred3->select(p1, ctx));
    ASSERT_FALSE(pred4->select(p1, ctx));

    ASSERT_FALSE(pred->select(p2, ctx));
    ASSERT_FALSE(pred2->select(p2, ctx));
    ASSERT_TRUE(pred3->select(p2, ctx));
    ASSERT_TRUE(pred4->select(p2, ctx));

	ASSERT_TRUE(pred->select_alive(p1, ctx));
	ASSERT_TRUE(pred2->select_alive(p1, ctx));
	ASSERT_FALSE(pred3->select_alive(p1, ctx));
	ASSERT_FALSE(pred4->select_alive(p1, ctx));

	ASSERT_FALSE(pred->select_alive(p2, ctx));
	ASSERT_FALSE(pred2->select_alive(p2, ctx));
	ASSERT_TRUE(pred3->select_alive(p2, ctx));
	ASSERT_TRUE(pred4->select_alive(p2, ctx));
}

TEST(PredicateFactory, PredPregnancyAtEnd) {
	std::shared_ptr<const Predicate<Person> > pred = PredicateFactory::make_pregnancy(Pregnancy::State::NOT_PREGNANT, true, false);
	ASSERT_EQ("Pregnancy(0, 1, 0)", get_name(*pred));
	Person p1(1, PersonAttributes(Sex::FEMALE, 0), Date(1990, 1, 1));
	p1.set_histories(build_int_histories());

	const Date d1(2012, 1, 1);
	const Date d2(2012, 6, 1);
	auto imm_ctx = std::make_shared<ImmutableContext>(Schedule({ d1, d2 }));
	const auto idx = imm_ctx->register_person_variable(Procreation::PREGNANCY_EVENT(), DISPATCHER_PREGNANCY);
	auto mut_ctx = std::make_shared<MutableContext>();
	Contexts ctx(imm_ctx, mut_ctx);

	p1.history(idx).append_int(Date(2011, 6, 1), static_cast<int>(Pregnancy::Event::CONCEPTION));
	ASSERT_FALSE(pred->select(p1, ctx));
	p1.history(idx).append_int(Date(2012, 3, 1), static_cast<int>(Pregnancy::Event::BIRTH));
	ASSERT_TRUE(pred->select(p1, ctx));

	ctx.mutable_ctx().advance_date_index();
	ASSERT_TRUE(pred->select(p1, ctx));
}

TEST(PredicateFactory, PredImmigrationDate) {
	Contexts ctx(Date(2015, 9, 16));
	auto pred1 = PredicateFactory::make_immigration_date(Date(2010, 1, 1), Date(2011, 1, 1), false, true);
	auto pred2 = PredicateFactory::make_immigration_date(Date(2010, 1, 1), Date(2011, 1, 1), true, true);
	auto pred1b = PredicateFactory::make_immigration_date(Date(2010, 1, 1), Date(2011, 1, 1), false, false);
	auto pred2b = PredicateFactory::make_immigration_date(Date(2010, 1, 1), Date(2011, 1, 1), true, false);
	ASSERT_FALSE(pred1->active(Date(2009, 12, 31)));
	ASSERT_TRUE(pred1->active(Date(2010, 1, 1)));
	ASSERT_TRUE(pred2->active(Date(2009, 12, 31)));
	ASSERT_TRUE(pred2->active(Date(2010, 1, 1)));
	ASSERT_FALSE(pred1b->active(Date(2009, 12, 31)));
	ASSERT_TRUE(pred1b->active(Date(2010, 1, 1)));
	ASSERT_TRUE(pred2b->active(Date(2009, 12, 31)));
	ASSERT_TRUE(pred2b->active(Date(2010, 1, 1)));
	Person p1(1, PersonAttributes(Sex::FEMALE, 0), Date(1990, 1, 1));
	ASSERT_FALSE(pred1->select_out_of_context(p1));
	ASSERT_TRUE(pred2->select_out_of_context(p1));
	ASSERT_FALSE(pred1b->select_out_of_context(p1));
	ASSERT_TRUE(pred2b->select_out_of_context(p1));
	ASSERT_FALSE(pred1->select(p1, ctx));
	ASSERT_TRUE(pred2->select(p1, ctx));
	ASSERT_FALSE(pred1b->select(p1, ctx));
	ASSERT_TRUE(pred2b->select(p1, ctx));
	ASSERT_FALSE(pred1->select_alive(p1, ctx));
	ASSERT_TRUE(pred2->select_alive(p1, ctx));
	ASSERT_FALSE(pred1b->select_alive(p1, ctx));
	ASSERT_TRUE(pred2b->select_alive(p1, ctx));
	p1.set_immigration_date(Date(2008, 6, 1));
	ASSERT_FALSE(pred1->select_out_of_context(p1));
	ASSERT_FALSE(pred2->select_out_of_context(p1));
	ASSERT_FALSE(pred1b->select_out_of_context(p1));
	ASSERT_FALSE(pred2b->select_out_of_context(p1));
	ASSERT_FALSE(pred1->select(p1, ctx));
	ASSERT_FALSE(pred2->select(p1, ctx));
	ASSERT_FALSE(pred1b->select(p1, ctx));
	ASSERT_FALSE(pred2b->select(p1, ctx));

	ASSERT_FALSE(pred1->select_alive(p1, ctx));
	ASSERT_FALSE(pred2->select_alive(p1, ctx));
	ASSERT_FALSE(pred1b->select_alive(p1, ctx));
	ASSERT_FALSE(pred2b->select_alive(p1, ctx));

	Person p2(1, PersonAttributes(Sex::FEMALE, 0), Date(1990, 1, 1));
	p2.set_immigration_date(Date(2011, 7, 1));
	ASSERT_FALSE(pred1->select_out_of_context(p2));
	ASSERT_FALSE(pred2->select_out_of_context(p2));
	ASSERT_FALSE(pred1b->select_out_of_context(p2));
	ASSERT_FALSE(pred2b->select_out_of_context(p2));
	ASSERT_FALSE(pred1->select(p2, ctx));
	ASSERT_FALSE(pred2->select(p2, ctx));
	ASSERT_FALSE(pred1b->select(p2, ctx));
	ASSERT_FALSE(pred2b->select(p2, ctx));
	Person p3(1, PersonAttributes(Sex::FEMALE, 0), Date(1990, 1, 1));
	p3.set_immigration_date(Date(2010, 7, 1));
	ASSERT_TRUE(pred1->select_out_of_context(p3));
	ASSERT_TRUE(pred2->select_out_of_context(p3));
	ASSERT_TRUE(pred1b->select_out_of_context(p3));
	ASSERT_TRUE(pred2b->select_out_of_context(p3));
	ASSERT_TRUE(pred1->select(p3, ctx));
	ASSERT_TRUE(pred2->select(p3, ctx));
	ASSERT_TRUE(pred1b->select(p3, ctx));
	ASSERT_TRUE(pred2b->select(p3, ctx));
	p3.die(Date(2014, 9, 30));
	ASSERT_TRUE(pred1->select_out_of_context(p3));
	ASSERT_TRUE(pred2->select_out_of_context(p3));
	ASSERT_TRUE(pred1b->select_out_of_context(p3));
	ASSERT_TRUE(pred2b->select_out_of_context(p3));
	ASSERT_FALSE(pred1->select(p3, ctx));
	ASSERT_FALSE(pred2->select(p3, ctx));
	ASSERT_TRUE(pred1b->select(p3, ctx));
	ASSERT_TRUE(pred2b->select(p3, ctx));

	ASSERT_TRUE(pred1->select_alive(p3, ctx)); // because we don't check is_alive
	ASSERT_TRUE(pred2->select_alive(p3, ctx)); // because we don't check is_alive
	ASSERT_TRUE(pred1b->select_alive(p3, ctx));
	ASSERT_TRUE(pred2b->select_alive(p3, ctx));
}

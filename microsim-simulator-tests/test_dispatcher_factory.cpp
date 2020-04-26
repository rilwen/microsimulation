// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "mock_predicate.hpp"
#include "microsim-simulator/dispatcher_factory.hpp"
#include "microsim-simulator/predicate_factory.hpp"
#include "microsim-simulator/contexts.hpp"
#include "microsim-simulator/operator_individual.hpp"

using namespace averisera;
using namespace averisera::microsim;

class MockFunctor: public Functor<std::pair<int, double>, double> {
public:
    double operator()(const std::pair<int, double>& obj, const Contexts&) const {
        return obj.first * obj.second;
    }
    
    const FeatureUser<Feature>::feature_set_t& requires() const {
        return Feature::empty();
    }
};

class MockOperator: public OperatorIndividual<int> {
public:
    MockOperator(int tgt, int multi)
    : OperatorIndividual<int>(true), _pred(tgt), _multi(multi) {}
    
    const Predicate<int>& predicate() const {
        return _pred;
    }
    
    void apply(const std::shared_ptr<int>& obj, const Contexts&) const {
        (*obj) *= _multi;
    }

	const std::string& name() const override {
		static const std::string str("Mock");
		return str;
	}
private:
    MockPredicate _pred;
    int _multi;
};

TEST(DispatcherFactory, Range1D) {
    std::vector<double> thresholds = {0, 10};
    std::unique_ptr<const Dispatcher<std::pair<int, double>, unsigned int>> disp(DispatcherFactory::make_range_1d<std::pair<int, double>>(std::make_shared<MockFunctor>(), thresholds));
    ASSERT_NE(nullptr, disp);
    Contexts ctx;
    ASSERT_NE(nullptr, disp->predicate());
    ASSERT_TRUE(disp->predicate()->always_true());
    ASSERT_TRUE(disp->predicate()->select(std::make_pair(2, 20.0), ctx));
    ASSERT_EQ(2u, disp->dispatch(std::make_pair(2, 20.0), ctx));
    ASSERT_EQ(1u, disp->dispatch(std::make_pair(0, 20.0), ctx));
    ASSERT_EQ(0u, disp->dispatch(std::make_pair(-2, 20.0), ctx));
    ASSERT_EQ(1u, disp->dispatch(std::make_pair(1, 5.0), ctx));
    ASSERT_EQ(2u, disp->dispatch(std::make_pair(1, 10.0), ctx));

    ASSERT_EQ(0u, disp->dispatch_out_of_context(std::make_pair(2, 20.0)));
    ASSERT_EQ(0u, disp->dispatch_out_of_context(std::make_pair(0, 20.0)));
    ASSERT_EQ(0u, disp->dispatch_out_of_context(std::make_pair(-2, 20.0)));
    ASSERT_EQ(0u, disp->dispatch_out_of_context(std::make_pair(1, 5.0)));
    ASSERT_EQ(0u, disp->dispatch_out_of_context(std::make_pair(1, 10.0)));
}

TEST(DispatcherFactory, OperatorGroup) {
    std::vector<std::shared_ptr<const Operator<int>>> operators(2);
    operators[0] = std::make_shared<MockOperator>(0, 2);
    operators[1] = std::make_shared<MockOperator>(1, -2);
    std::shared_ptr<const Predicate<int>> group_pred = PredicateFactory::make_or<int>({std::make_shared<MockPredicate>(0), std::make_shared<MockPredicate>(1)});
    std::unique_ptr<const Dispatcher<int, unsigned int>> disp(DispatcherFactory::make_operator_group(operators, group_pred));
    ASSERT_NE(nullptr, disp);
    Contexts ctx;
    ASSERT_EQ(0u, disp->requires().size());
    ASSERT_NE(nullptr, disp->predicate());
    ASSERT_FALSE(disp->predicate()->always_true());
    ASSERT_FALSE(disp->predicate()->always_true_out_of_context());
    ASSERT_TRUE(disp->predicate()->select(0, ctx));
    ASSERT_FALSE(disp->predicate()->select(2, ctx));
    ASSERT_EQ(0u, disp->dispatch(0, ctx));
    ASSERT_EQ(1u, disp->dispatch(1, ctx));
    ASSERT_THROW(disp->dispatch(2, ctx), std::domain_error);
    ASSERT_EQ(0u, disp->dispatch_out_of_context(0));
    ASSERT_EQ(1u, disp->dispatch_out_of_context(1));
    ASSERT_THROW(disp->dispatch_out_of_context(2), std::domain_error);
}

TEST(DispatcherFactory, Constant) {
    const std::shared_ptr<const Predicate<int>> pred = PredicateFactory::make_true<int>();
    std::unique_ptr<const Dispatcher<int, int>> disp(DispatcherFactory::make_constant<int, int>(-2, pred));
    ASSERT_NE(nullptr, disp);
    Contexts ctx;
    ASSERT_EQ(-2, disp->dispatch(1011, ctx));
    ASSERT_EQ(-2, disp->dispatch_out_of_context(1011));
    ASSERT_NE(nullptr, disp->predicate());
    ASSERT_TRUE(disp->predicate()->always_true());
}

TEST(DispatcherFactory, Group) {
    std::vector<std::shared_ptr<const Predicate<int>>> predicates(3);
    const std::vector<double> values = { -0.1, 0.0, 0.2 };
    predicates[0] = std::make_shared<MockPredicate>(-2);
    predicates[1] = std::make_shared<MockPredicate>(4);
    predicates[2] = PredicateFactory::make_true<int>();
    const FeatureUser<Feature>::feature_set_t reqs({ "Foo" });
    Contexts ctx;
    std::unique_ptr<const Dispatcher<int, double>> disp1(DispatcherFactory::make_group(predicates, values, reqs, predicates[2]));
    ASSERT_NE(nullptr, disp1);
    ASSERT_NE(nullptr, disp1->predicate());
    ASSERT_TRUE(disp1->predicate()->always_true());
    ASSERT_TRUE(disp1->predicate()->select(-2, ctx));
    ASSERT_TRUE(disp1->predicate()->select(100, ctx));
    ASSERT_EQ(reqs, disp1->requires());
    ASSERT_EQ(values[0], disp1->dispatch(-2, ctx));
    ASSERT_EQ(values[1], disp1->dispatch(4, ctx));
    ASSERT_EQ(values[2], disp1->dispatch(100, ctx));
    ASSERT_TRUE(disp1->predicate()->always_true_out_of_context());
    ASSERT_TRUE(disp1->predicate()->select_out_of_context(-2));
    ASSERT_TRUE(disp1->predicate()->select_out_of_context(100));
    ASSERT_EQ(values[0], disp1->dispatch_out_of_context(-2));
    ASSERT_EQ(values[1], disp1->dispatch_out_of_context(4));
    ASSERT_EQ(values[2], disp1->dispatch_out_of_context(100));
    
    std::unique_ptr<const Dispatcher<int, double>> disp2(DispatcherFactory::make_group(predicates, values, reqs));
    ASSERT_NE(nullptr, disp2);
    ASSERT_NE(nullptr, disp2->predicate());
    ASSERT_TRUE(disp2->predicate()->always_true());
    ASSERT_TRUE(disp2->predicate()->select(-2, ctx));
    ASSERT_TRUE(disp2->predicate()->select(100, ctx));
    ASSERT_EQ(reqs, disp2->requires());
    ASSERT_EQ(values[0], disp2->dispatch(-2, ctx));
    ASSERT_EQ(values[1], disp2->dispatch(4, ctx));
    ASSERT_EQ(values[2], disp2->dispatch(100, ctx));
    predicates[2] = std::make_shared<MockPredicate>(6);
    std::unique_ptr<const Dispatcher<int, double>> disp3(DispatcherFactory::make_group(predicates, values, reqs));
    ASSERT_NE(nullptr, disp3);
    ASSERT_NE(nullptr, disp3->predicate());
    ASSERT_FALSE(disp3->predicate()->always_true());
    ASSERT_FALSE(disp3->predicate()->always_true_out_of_context());
    ASSERT_TRUE(disp3->predicate()->select(-2, ctx));
    ASSERT_TRUE(disp3->predicate()->select(6, ctx));
    ASSERT_FALSE(disp3->predicate()->select(100, ctx));
    ASSERT_EQ(reqs, disp3->requires());
    ASSERT_EQ(values[0], disp3->dispatch(-2, ctx));
    ASSERT_EQ(values[1], disp3->dispatch(4, ctx));
    ASSERT_EQ(values[2], disp3->dispatch(6, ctx));
    ASSERT_THROW(disp3->dispatch(100, ctx), std::domain_error);
    ASSERT_THROW(disp3->dispatch_out_of_context(100), std::domain_error);
}

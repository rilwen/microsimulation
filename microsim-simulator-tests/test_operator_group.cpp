#include <gtest/gtest.h>
#include "microsim-simulator/contexts.hpp"
#include "microsim-simulator/operator_group.hpp"
#include "microsim-simulator/predicate_factory.hpp"
#include "mock_operator.hpp"
#include "mock_predicate.hpp"

using namespace averisera;
using namespace averisera::microsim;

typedef MockOperator<MockPredicate, int> OurOperator;

class MockDispatcher: public Dispatcher<int, unsigned int> {
public:
    MockDispatcher() {
        _pred = PredicateFactory::make_true<int>();
    }

    unsigned int dispatch(const int& obj, const Contexts& contexts) const override {
        return static_cast<unsigned int>(obj);
    }

    unsigned int dispatch_out_of_context(const int& obj) const override {
        return static_cast<unsigned int>(obj);
    }

    const FeatureUser<Feature>::feature_set_t& requires() const override {
        static const FeatureUser<Feature>::feature_set_t EMPTY;
        return EMPTY;
    }

    std::shared_ptr<const Predicate<int> > predicate() const override {
        return _pred;
    }
private:
    std::shared_ptr<const Predicate<int> > _pred;
};

TEST(OperatorGroup, Constructor) {
    std::vector<std::shared_ptr<const Operator<int> > > operators(3);
    operators[0] = std::make_shared<OurOperator>(MockPredicate(0), true);
    operators[1] = std::make_shared<OurOperator>(MockPredicate(1), true);
    operators[2] = std::make_shared<OurOperator>(MockPredicate(2), true);
    auto disp = std::make_shared<MockDispatcher>();
    OperatorGroup<int> opgrp(operators, disp);
    ASSERT_EQ(&*disp->predicate(), &opgrp.predicate());
    std::vector<std::shared_ptr<int> > objects(4, std::make_shared<int>(1));
    Contexts ctx;
    opgrp.apply(objects, ctx);
}

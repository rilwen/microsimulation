// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MICROSIM_TEST_MOCK_OPERATOR_HPP
#define __AVERISERA_MICROSIM_TEST_MOCK_OPERATOR_HPP

#include "microsim-simulator/operator.hpp"

namespace averisera {
    namespace microsim {
        typedef FeatureUser<Feature>::feature_set_t vf;
        
        template <class Pred, class Obj> class MockOperator: public Operator<Obj> {
            public:
            MockOperator(bool is_inst)
                : MockOperator<Pred, Obj>(Pred(), is_inst, vf(), vf()) {
            }
            
            MockOperator(bool is_inst, const vf& provides, const vf& requires)
                : MockOperator<Pred, Obj>(Pred(), is_inst, provides, requires) {
            }
            
            MockOperator(const Pred& pred, bool is_inst)
                : MockOperator<Pred, Obj>(pred, is_inst, vf(), vf()) {
            }
            
            MockOperator(const Pred& pred, bool is_inst, const vf& provides, const vf& requires)
                : Operator<Obj>(is_inst, provides, requires), _pred(pred) {
            }
            
            const Predicate<Obj>& predicate() const {
                return _pred;
            }
            
            void apply(const std::vector<std::shared_ptr<Obj>>&, const Contexts&) const {
            }

			const std::string& name() const override {
				static const std::string str("Mock");
				return str;
			}
        private:
            Pred _pred;            
        };
    }
}

#endif // __AVERISERA_MICROSIM_TEST_MOCK_OPERATOR_HPP

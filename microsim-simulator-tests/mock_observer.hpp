#ifndef __AVERISERA_MICROSIM_TEST_MOCK_OBSERVER_HPP
#define __AVERISERA_MICROSIM_TEST_MOCK_OBSERVER_HPP

#include "microsim-simulator/observer.hpp"

namespace averisera {
    namespace microsim {
        class MockObserver: public Observer {
        public:
            MockObserver()
                : Observer(nullptr) {}

        private:
            
            void observe(const Population& population, const Contexts& ctx) override {
            }

            void save_results(std::ostream& os, const ImmutableContext& im_ctx) const override {
            }
        };
    }
}

#endif // __AVERISERA_MICROSIM_TEST_MOCK_OBSERVER_HPP

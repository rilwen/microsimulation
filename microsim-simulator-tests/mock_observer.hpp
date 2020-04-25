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
            
            void observe(const Population&, const Contexts&) override {
            }

            void save_results(std::ostream&, const ImmutableContext&) const override {
            }
        };
    }
}

#endif // __AVERISERA_MICROSIM_TEST_MOCK_OBSERVER_HPP

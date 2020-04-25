#ifndef __AVERISERA_MICROSIM_TEST_MOCK_PREDICATE_H
#define __AVERISERA_MICROSIM_TEST_MOCK_PREDICATE_H

#include "microsim-simulator/predicate.hpp"
#include "core/dates.hpp"

namespace averisera {
    namespace microsim {
        /** Used in several tests */
        class MockPredicate: public Predicate<int> {
        public:
            MockPredicate(int n, bool active = true)
                : _n(n), active_(active) {}
            
            bool select(const int& obj, const Contexts&) const {
                return obj == _n;
            }

            bool select_out_of_context(const int& obj) const {
                return active_ && (obj == _n);
            }
            
            MockPredicate* clone() const override {
                return new MockPredicate(_n);
            }

			void print(std::ostream& os) const override {
				os << "Mock(" << _n << ")";
			}

			bool active(Date) const override {
				return active_;
			}
        private:
            int _n;
			bool active_;
        };
    }
}

#endif // __AVERISERA_MICROSIM_TEST_MOCK_PREDICATE_H

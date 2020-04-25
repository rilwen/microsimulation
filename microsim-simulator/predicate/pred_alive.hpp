#ifndef __AVERISERA_MICROSIM_PRED_ALIVE_H
#define __AVERISERA_MICROSIM_PRED_ALIVE_H

#include "../predicate.hpp"
#include <ostream>

namespace averisera {
    namespace microsim {
        class Person;

        /**  Selects Person objects which are alive */
        class PredAlive: public Predicate<Person> {
        public:
            bool select(const Person& obj, const Contexts& contexts) const override;

			bool select_alive(const Person& obj, const Contexts& contexts) const override {
				return true;
			}

            PredAlive* clone() const override {
                return new PredAlive();
            }

            bool select_out_of_context(const Person& obj) const override {
                return true;
            }

            bool always_true_out_of_context() const override {
                return true;
            }

			void print(std::ostream& os) const override {
				os << "Alive";
			}

			bool selects_alive_only() const override {
				return true;
			}
        };
    }
}

#endif // __AVERISERA_MICROSIM_PRED_ALIVE_H

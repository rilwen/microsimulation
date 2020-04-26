// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MICROSIM_PRED_PREGNANCY_H
#define __AVERISERA_MICROSIM_PRED_PREGNANCY_H

#include "../person.hpp"
#include "../predicate.hpp"
#include "../procreation.hpp"
#include "microsim-core/pregnancy.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        class Person;

        /** Predicate selecting given pregnancy state. Selects only females. */
        class PredPregnancy: public Predicate<Person> {
        public:
            /** @param state Selected state
              @param alive Whether to select only alive persons
			  @param at_start Whether to check at the beginning of the schedule period (true) or at the end (false)
              @throw std::domain_error If state == State::SIZE
             */
            PredPregnancy(const Pregnancy::State state, bool alive, bool at_start);

            bool select(const Person& obj, const Contexts& contexts) const override;

			bool select_alive(const Person& obj, const Contexts& contexts) const override;

            bool select_out_of_context(const Person& obj) const override;

            bool always_true() const override {
                return false;
            }

            bool always_true_out_of_context() const override {
                return false;
            }

            PredPregnancy* clone() const override {
                return new PredPregnancy(_state, _alive, at_start_);
            }

			void print(std::ostream& os) const override;

			bool selects_alive_only() const override {
				return _alive;
			}
        private:
			bool is_pregnant(const Person& obj, const Contexts& contexts) const;

            Pregnancy::State _state;
            bool _alive;
			bool at_start_;
        };
    }
}

#endif // __AVERISERA_MICROSIM_PRED_PREGNANCY_H

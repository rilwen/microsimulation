/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_PRED_AGE_H
#define __AVERISERA_MS_PRED_AGE_H

#include "../predicate.hpp"

namespace averisera {
    namespace microsim {
        class Person;
        
        /** @brief Selects Person which has age (in years) within a given range (inclusive on both sides). */
        class PredAge: public Predicate<Person> {
        public:
            /** @param[in] min_age Minimum age
             * @param[in] max_age Maximum age
             @param[in] alive Person must be alive
             * @throw std::domain_error If min_age > max_age
             */
            PredAge(unsigned int min_age, unsigned int max_age, bool alive = true);
            
            bool select(const Person& obj, const Contexts& contexts) const override;

			bool select_alive(const Person& obj, const Contexts& contexts) const override;

            bool select_out_of_context(const Person& /*obj*/) const override {
                return true;
            }

            PredAge* clone() const override {
                return new PredAge(_min_age, _max_age, _alive);
            }

            bool always_true_out_of_context() const override {
                return true;
            }

			void print(std::ostream& os) const override;

			bool selects_alive_only() const override {
				return _alive;
			}
        private:            
            unsigned int _min_age;
            unsigned int _max_age;
            bool _alive;			

			bool select_alive_impl(const Person& obj, Date asof) const;
        };
    }
}

#endif // __AVERISERA_MS_PRED_AGE_H

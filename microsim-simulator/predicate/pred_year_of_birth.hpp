/*
(C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_MS_PRED_YEAR_OF_BIRTH_H
#define __AVERISERA_MS_PRED_YEAR_OF_BIRTH_H

#include "../predicate.hpp"

namespace averisera {
	namespace microsim {
		class Person;

		/** Select Person which has year of birth in given range.
		*/
		class PredYearOfBirth : public Predicate < Person > {
		public:
			/**
			@param[in] min_year Minimum year of birth to select.
			@param[in] max_year Maximum year of birth to select.
            @param[in] alive Person must be alive
			@throw std::domain_error If min_year > max_year
			*/
			PredYearOfBirth(int min_year, int max_year, bool alive = true);
                        
			bool select(const Person& obj, const Contexts& contexts) const override;

			bool select_alive(const Person& obj, const Contexts&) const override {
				return select_out_of_context(obj);
			}

            bool select_out_of_context(const Person& obj) const override;

            PredYearOfBirth* clone() const override {
                return new PredYearOfBirth(_min_yob, _max_yob, _alive);
            }

			void print(std::ostream& os) const override;

			bool selects_alive_only() const override {
				return _alive;
			}
		private:
			int _min_yob;
			int _max_yob;
            bool _alive;
		};
	}
}

#endif // __AVERISERA_MS_PRED_YEAR_OF_BIRTH_H

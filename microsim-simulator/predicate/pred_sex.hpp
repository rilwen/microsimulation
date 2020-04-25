/*
(C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_MS_PRED_SEX_H
#define __AVERISERA_MS_PRED_SEX_H

#include "../predicate.hpp"
#include <cstdint>

namespace averisera {
	namespace microsim {
		class Person;
		enum class Sex: uint8_t;

		/** Select Person with given sex
		*/
		class PredSex : public Predicate < Person > {
		public:
            /** @param[in] Person must be alive */
			PredSex(Sex sex, bool alive = true);
			bool select(const Person& obj, const Contexts& contexts) const override;
			bool select_alive(const Person& obj, const Contexts& contexts) const override {
				return select_out_of_context(obj);
			}
            bool select_out_of_context(const Person& obj) const override;
            PredSex* clone() const override {
                return new PredSex(_sex, _alive);
            }
			void print(std::ostream& os) const override;
			bool selects_alive_only() const override {
				return _alive;
			}
		private:
			Sex _sex;
            bool _alive;
		};
	}
}

#endif // __AVERISERA_MS_PRED_SEX_H

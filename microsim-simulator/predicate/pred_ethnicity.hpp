/*
(C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_MS_PRED_ETHNICITY_H
#define __AVERISERA_MS_PRED_ETHNICITY_H

#include "../predicate.hpp"
#include "microsim-core/person_attributes.hpp"

namespace averisera {
	namespace microsim {
		class Person;

		/** Select Person with ethnicity belonging to some set of values.
		*/
		class PredEthnicity : public Predicate < Person > {
		public:
			typedef PersonAttributes::ethnicity_t ethnicity_type;

            /* @param[in] alive Person must be alive  */
			PredEthnicity(bool alive);

			bool select(const Person& obj, const Contexts& contexts) const override;

			bool select_alive(const Person& obj, const Contexts& contexts) const override {
				return select_out_of_context(obj);
			}

			void print(std::ostream& os) const override;

			bool selects_alive_only() const override {
				return get_alive();
			}
		protected:
			bool get_alive() const {
				return !accept_dead_;
			}
		private:
            bool accept_dead_;		

			bool life_signs_ok(const Person& obj, const Contexts& contexts) const;

			/** Return sorted vector of selected ethnic group indices as int values */
			virtual std::vector<int> get_group_indices() const = 0;

			/** Suffix naming the method of ethnicity selection */
			virtual const char* get_name_suffix() const = 0;
		};
	}
}

#endif // __AVERISERA_MS_PRED_ETHNICITY_H

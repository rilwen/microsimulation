/*
(C) Averisera Ltd 2015
*/

#include "pred_year_of_birth.hpp"
#include "core/dates.hpp"
#include "microsim-simulator/person.hpp"
#include "../contexts.hpp"
#include <stdexcept>

namespace averisera {
	namespace microsim {
		PredYearOfBirth::PredYearOfBirth(int min_year, int max_year, bool alive)
            : _alive(alive) {
			if (min_year > max_year) {
				throw std::domain_error("PredYearOfBirth: Min year higher than max year");
			}
			_min_yob = min_year;
			_max_yob = max_year;          
		}

		bool PredYearOfBirth::select(const Person& obj, const Contexts& contexts) const {
            if (_alive && (!obj.is_alive(contexts.asof()))) {
                return false;
            }
			return select_out_of_context(obj);
		}

        bool PredYearOfBirth::select_out_of_context(const Person& obj) const {
			const int yob = obj.year_of_birth();
			return _min_yob <= yob && yob <= _max_yob;
        }

		void PredYearOfBirth::print(std::ostream& os) const {
			os << "YearOfBirth(" << _min_yob << ", " << _max_yob << ", " << _alive << ")";
		}
	}
}

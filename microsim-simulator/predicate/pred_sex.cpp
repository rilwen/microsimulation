/*
(C) Averisera Ltd 2015
*/
#include "pred_sex.hpp"
#include "../person.hpp"
#include "../contexts.hpp"

namespace averisera {
	namespace microsim {
		PredSex::PredSex(Sex sex, bool alive)
			: _sex(sex), _alive(alive) {
		}

		bool PredSex::select(const Person& obj, const Contexts& contexts) const {
            if (_alive && (!obj.is_alive(contexts.asof()))) {
                return false;
            }
			return select_out_of_context(obj);
		}

        bool PredSex::select_out_of_context(const Person& obj) const {
            return _sex == obj.sex();
        }

		void PredSex::print(std::ostream& os) const {
			os << "Sex(" << _sex << ", " << _alive << ")";
		}
	}
}

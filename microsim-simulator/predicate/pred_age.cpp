/*
 * (C) Averisera Ltd 2015
 */
#include "pred_age.hpp"
#include <stdexcept>
#include "../person.hpp"
#include "../contexts.hpp"
#include <boost/format.hpp>

namespace averisera {
    namespace microsim {
        PredAge::PredAge(unsigned int min_age, unsigned max_age, bool alive)
            : _min_age(min_age), _max_age(max_age), _alive(alive) {
            if (min_age > max_age) {
                throw std::domain_error("PredAge: minimum age above maximum age");
            }
        }
        
        bool PredAge::select(const Person& obj, const Contexts& contexts) const {
            const Date asof = contexts.asof();
            if (_alive && (!obj.is_alive(asof))) {
                return false;
            }
			return select_alive_impl(obj, asof);
        }

		bool PredAge::select_alive(const Person& obj, const Contexts& contexts) const {
			const Date asof = contexts.asof();
			assert(obj.is_alive(asof));
			return select_alive_impl(obj, asof);
		}

		void PredAge::print(std::ostream& os) const {
			os << "Age(" << _min_age << ", " << _max_age << ", " << _alive << ")";
		}

		bool PredAge::select_alive_impl(const Person& obj, Date asof) const {
			if (asof >= obj.date_of_birth()) {
				const unsigned int age = obj.age(asof);
				return age >= _min_age && age <= _max_age;
			} else {
				return false;
			}
		}
    }
}

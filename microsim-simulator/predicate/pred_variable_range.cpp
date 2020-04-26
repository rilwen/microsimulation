// (C) Averisera Ltd 2014-2020
#include "pred_variable_range.hpp"
#include "../actor.hpp"
#include "../contexts.hpp"
//#include "../history_registry.h"
#include "../history.hpp"
#include "../person.hpp"
#include <limits>
#include <stdexcept>

namespace averisera {
    namespace microsim {

        template <class V> static bool is_always_true(V min, V max, bool accept_missing);

        template <> bool is_always_true<History::double_t>(History::double_t min, History::double_t max, bool accept_missing) {
            return accept_missing && (min == -std::numeric_limits<History::double_t>::infinity()) && (max == std::numeric_limits<History::double_t>::infinity());
        }

        template <> bool is_always_true<History::int_t>(History::int_t min, History::int_t max, bool accept_missing) {
            return accept_missing && (min == std::numeric_limits<History::int_t>::min()) && (max == std::numeric_limits<History::int_t>::max());
        }

        template <class T, class V> PredVariableRange<T, V>::PredVariableRange(const std::string& variable, V min, V max, bool accept_missing)
            : _variable(variable), _min(min), _max(max), _accept_missing(accept_missing), _always_true(is_always_true(min, max, accept_missing)) {
            if (_variable.empty()) {
                throw std::domain_error("PredVariableRange: variable name empty");
            }
            if (_min > _max) {
                throw std::domain_error("PredVariableRange: range limits out of order");
            }
        }

        template <class V> static V get_last_value(const ImmutableHistory& history, Date date);

        template <> History::double_t get_last_value<History::double_t>(const ImmutableHistory& history, Date date) {
            return history.last_as_double(date);
        }

        template <> History::int_t get_last_value<History::int_t>(const ImmutableHistory& history, Date date) {
            return history.last_as_int(date);
        }        

        template <class T, class V> bool PredVariableRange<T, V>::select(const T& obj, const Contexts& contexts) const {
            const auto hist_idx = obj.get_variable_index(contexts, _variable);
            const ImmutableHistory& history = obj.history(hist_idx);
            const Date asof = contexts.asof();
            if (history.empty() || history.first_date() > asof) {
                return _accept_missing;
            } else {
                const V val = get_last_value<V>(history, asof);
                return _min <= val && val <= _max;
            }
        }

		template <class T, class V> void PredVariableRange<T, V>::print(std::ostream& os) const {
			os << "VariableRange(\"" << _variable << "\", " << _min << ", " << _max << ", " << _accept_missing << ")";
		}

        template class PredVariableRange<Actor, History::double_t>;
        template class PredVariableRange<Person, History::double_t>;
        template class PredVariableRange<Actor, History::int_t>;
        template class PredVariableRange<Person, History::int_t>;
    }
}

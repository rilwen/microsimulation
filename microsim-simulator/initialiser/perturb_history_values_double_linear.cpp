#include "perturb_history_values_double_linear.hpp"
#include "../contexts.hpp"
#include "../history_data.hpp"
#include "../mutable_context.hpp"
#include "../person_data.hpp"
#include "core/distribution.hpp"
#include "core/rng.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        template <class AD> PerturbHistoryValuesDoubleLinear<AD>::PerturbHistoryValuesDoubleLinear(const std::string& variable_name, double lower_bound, double upper_bound, std::unique_ptr<const Distribution>&& distr)
            : PerturbHistoryValuesDouble<AD>(variable_name, lower_bound, upper_bound), _distr(std::move(distr)) {
            if (!_distr) {
                throw std::domain_error("PerturbHistoryValuesDoubleLinear: null distribution");
            }
        }
        
        template <class AD> double PerturbHistoryValuesDoubleLinear<AD>::perturb(Date date, double value, const Contexts& ctx) const {
            return value + _distr->draw(ctx.mutable_ctx().rng());
        }

        template class PerturbHistoryValuesDoubleLinear<ActorData>;
        template class PerturbHistoryValuesDoubleLinear<PersonData>;
    }
}

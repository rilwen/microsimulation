// (C) Averisera Ltd 2014-2020
#include "perturb_history_values_double_logarithmic.hpp"
#include "../contexts.hpp"
#include "../history_data.hpp"
#include "../mutable_context.hpp"
#include "../person_data.hpp"
#include "core/distribution.hpp"
#include "core/rng.hpp"
#include <cmath>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        template <class AD> PerturbHistoryValuesDoubleLogarithmic<AD>::PerturbHistoryValuesDoubleLogarithmic(const std::string& variable_name, double lower_bound, double upper_bound, std::unique_ptr<const Distribution>&& distr)
            : PerturbHistoryValuesDouble<AD>(variable_name, lower_bound, upper_bound), _distr(std::move(distr)) {
            if (!_distr) {
                throw std::domain_error("PerturbHistoryValuesDoubleLogarithmic: null distribution");
            }
        }
        
        template <class AD> double PerturbHistoryValuesDoubleLogarithmic<AD>::perturb(Date, double value, const Contexts& ctx) const {
            return value * exp(_distr->draw(ctx.mutable_ctx().rng()));
        }

        template class PerturbHistoryValuesDoubleLogarithmic<ActorData>;
        template class PerturbHistoryValuesDoubleLogarithmic<PersonData>;
    }
}

// (C) Averisera Ltd 2014-2020
#include "operator_incrementer_multi.hpp"
#include "core/multivariate_distribution.hpp"
#include "../person.hpp"
#include "../contexts.hpp"
#include "../immutable_context.hpp"
#include "../mutable_context.hpp"
#include <algorithm>

namespace averisera {
    namespace microsim {
        template <class T> OperatorIncrementerMulti<T>::OperatorIncrementerMulti(const std::vector<std::string>& variables, std::shared_ptr<const Predicate<T>> predicate, const std::vector<std::shared_ptr<const MultivariateDistribution>>& distributions, std::unique_ptr<Schedule>&& schedule)
            : OperatorIndividual<T>(false, Feature::from_names(variables)),
            history_user_(variables.begin(), variables.end()),
             _variables(variables), _predicate(predicate), _distributions(distributions) {
            if (!predicate) {
                throw std::domain_error("OperatorIncrementerMulti: null predicate");
            }
            if (std::any_of(distributions.begin(), distributions.end(), [](const std::shared_ptr<const MultivariateDistribution>& distr) { return !distr; })) {
                throw std::domain_error("OperatorIncrementerMulti: one or more distributions is null");
            }
            const unsigned int dim = static_cast<unsigned int>(variables.size());
            if (std::any_of(distributions.begin(), distributions.end(), [dim](const std::shared_ptr<const MultivariateDistribution>& distr) { return distr->dim() != dim; })) {
                throw std::domain_error("OperatorIncrementerMulti: one or more distributions has wrong dimension");
            }
            if (std::any_of(variables.begin(),variables.end(),[](const std::string& variable) { return variable.empty(); })) {
                throw std::domain_error("OperatorIncrementerMulti: empty variable name(s)");
            }            
            if (schedule && schedule->size() > _distributions.size()) {
                throw std::out_of_range("OperatorIncrementerMulti: not enough distributions");
            }
            _schedule = std::move(schedule);
        }
        
        template <class T> void OperatorIncrementerMulti<T>::apply(const std::shared_ptr<T>& actorptr, const Contexts& contexts) const {
            T& actor = *actorptr;
            if (!_schedule) {
                // custom schedule would have been checked in constructor
                if (contexts.immutable_ctx().schedule().size() > _distributions.size()) {
                    throw std::out_of_range("OperatorIncrementerMulti: not enough distributions");
                }
            } else {
                assert(contexts.immutable_ctx().schedule().contains(*_schedule));
            }

            const Date asof = contexts.asof();
            const Schedule& schedule = _schedule ? *_schedule : contexts.immutable_ctx().schedule();
            const Schedule::index_t idx = _schedule ? schedule.index(asof) : contexts.asof_idx();
            const unsigned int dim = this->dim();
            if (idx + 1 < schedule.nbr_dates()) {
                std::vector<double> deltas(dim);// TODO: avoid repated heap alloc
                const Date next_date = contexts.immutable_ctx().schedule().date(idx + 1);
                const MultivariateDistribution& distr = *(_distributions[idx]);
                const HistoryRegistry& registry = actor.get_history_registry(contexts.immutable_ctx());
                distr.draw(contexts.mutable_ctx().rng(), deltas);
                for (unsigned int k = 0; k < dim; ++k) {
                    const std::string& variable = _variables[k];
                    const auto hist_idx = registry.variable_index(variable);
                    History& history = actor.history(hist_idx);
                    const double prev = history.last_as_double(asof);
                    const double delta = deltas[k];
                    const double next = prev + delta;
                    history.append(next_date, next);
                }
            }
        }

        template class OperatorIncrementerMulti<Actor>;
        template class OperatorIncrementerMulti<Person>;
    }
}

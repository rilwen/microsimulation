// (C) Averisera Ltd 2014-2020
#include "operator_incrementer.hpp"
#include "core/distribution.hpp"
#include "../person.hpp"
#include "../contexts.hpp"
#include "../immutable_context.hpp"
#include "../mutable_context.hpp"
#include <cassert>

namespace averisera {
    namespace microsim {
        template <class T> OperatorIncrementer<T>::OperatorIncrementer(const std::string& variable, std::shared_ptr<const Predicate<T>> predicate, const std::vector<std::shared_ptr<const Distribution>>& distributions, std::unique_ptr<Schedule>&& schedule)
            : OperatorIndividual<T>(false, FeatureUser<Feature>::feature_set_t({variable})),
			history_user_(variable),
            _variable(variable), _predicate(predicate), _distributions(distributions)
        {
            if (!predicate) {
                throw std::domain_error("OperatorIncrementer: null predicate");
            }
            if (std::any_of(distributions.begin(), distributions.end(), [](const std::shared_ptr<const Distribution>& distr) { return !distr; })) {
                throw std::domain_error("OperatorIncrementer: one or more distributions is null");
            }
            if (variable.empty()) {
                throw std::domain_error("OperatorIncrementer: empty variable name");
            }
            if (schedule && schedule->size() > _distributions.size()) {
                throw std::out_of_range("OperatorIncrementer: not enough distributions");
            }
            _schedule = std::move(schedule);
        }
        
        template <class T> void OperatorIncrementer<T>::apply(const std::shared_ptr<T>& actorptr, const Contexts& contexts) const {
            T& actor = *actorptr;
            if (!_schedule) {
                // custom schedule would have been checked in constructor
                if (contexts.immutable_ctx().schedule().size() > _distributions.size()) {
                    throw std::out_of_range("OperatorIncrementer: not enough distributions");
                }
            } else {
                assert(contexts.immutable_ctx().schedule().contains(*_schedule));
            }
                
            const Date asof = contexts.asof();
            const Schedule& schedule = _schedule ? *_schedule : contexts.immutable_ctx().schedule();
            const Schedule::index_t idx = _schedule ? schedule.index(asof) : contexts.asof_idx();
            if (idx + 1 < schedule.nbr_dates()) {
                const Distribution& distr = *(_distributions[idx]);
                History& history = actor.history(contexts.immutable_ctx(), _variable);
                const double prev = history.last_as_double(asof);
                const double delta = distr.icdf(contexts.mutable_ctx().rng().next_uniform());
                const double next = prev + delta;
                const Date next_date = schedule.date(idx + 1);
                history.append(next_date, next);
            }
        }

        template class OperatorIncrementer<Actor>;
        template class OperatorIncrementer<Person>;
    }
}

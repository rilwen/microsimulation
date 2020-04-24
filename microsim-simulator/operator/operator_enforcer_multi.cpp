#include "operator_enforcer_multi.hpp"
#include <algorithm>
#include <stdexcept>
#include <Eigen/Core>
#include "core/multivariate_distribution.hpp"
#include "../contexts.hpp"
#include "../immutable_context.hpp"
#include "../mutable_context.hpp"
#include "../person.hpp"

namespace averisera {
    namespace microsim {
        template <class T> OperatorEnforcerMulti<T>::OperatorEnforcerMulti(const std::vector<std::string>& variables, std::shared_ptr<const Predicate<T>> predicate,
                                                                           const std::vector<std::shared_ptr<const MultivariateDistribution>>& distributions,
                                                                           const std::vector<HistoryFactory::factory_t>& history_factories,
                                                                           std::unique_ptr<Schedule>&& schedule)
            : Operator<T>(true, Feature::from_names(variables)),
            hist_gen_(variables.begin(), variables.end(), history_factories.begin(), history_factories.end(), predicate),
            _predicate(predicate), _distributions(distributions) {
            if (!predicate) {
                throw std::domain_error("OperatorEnforcerMulti: null predicate");
            }
            const unsigned int dim = static_cast<unsigned int>(variables.size());
            if (std::any_of(distributions.begin(), distributions.end(), [](const std::shared_ptr<const MultivariateDistribution>& distr) { return !distr; })) {
                throw std::domain_error("OperatorEnforcerMulti: one or more distributions is null");
            }
            if (std::any_of(distributions.begin(), distributions.end(), [dim](const std::shared_ptr<const MultivariateDistribution>& distr) { return distr->dim() != dim; })) {
                throw std::domain_error("OperatorEnforcerMulti: one or more distributions has wrong dimension");
            }
            if (std::any_of(variables.begin(),variables.end(),[](const std::string& variable) { return variable.empty(); })) {
                throw std::domain_error("OperatorEnforcerMulti: empty variable name(s)");
            }
            if (std::any_of(history_factories.begin(), history_factories.end(), [](HistoryFactory::factory_t factory) { return !factory; })) {
                throw std::domain_error("OperatorEnforcerMulti: one or more history factories is null");
            }
            if (schedule && schedule->nbr_dates() > _distributions.size()) {
                throw std::out_of_range("OperatorEnforcerMulti: not enough distributions for custom schedule");
            }
            _schedule = std::move(schedule);
        }
        
        template <class T> void OperatorEnforcerMulti<T>::apply(const std::vector<std::shared_ptr<T>>& selected, const Contexts& contexts) const {            
            if (!_schedule) {                
                // custom schedule would have been checked in constructor
                if (contexts.immutable_ctx().schedule().nbr_dates() > _distributions.size()) {
                    throw std::out_of_range("OperatorEnforcerMulti: not enough distributions");
                }
            } else {
                assert(contexts.immutable_ctx().schedule().contains(*_schedule));
            }
            const Date asof = contexts.asof();
            const Schedule::index_t idx = _schedule ? _schedule->index(asof) : contexts.asof_idx();
            const MultivariateDistribution& distr = *(_distributions[idx]);
            const size_t n = selected.size();
            const unsigned int dim = this->dim();
            Eigen::MatrixXd v(n, dim);            
            std::vector<History*> histories(n * dim);
            for (size_t i = 0; i < n; ++i) {      
                T& actor = *(selected[i]);
                const HistoryRegistry& registry = actor.get_history_registry(contexts.immutable_ctx());
                auto row = v.row(i);		
                for (unsigned int k = 0; k < dim; ++k) {
                    const std::string& variable = _variables[k];
                    const auto hist_idx = registry.variable_index(variable);
                    History& history = actor.history(hist_idx);
                    histories[i * dim + k] = &history; // histories are owned by the actor so they're not going away during the function call
                    //
                }
		if (std::all_of(histories.begin() + i * dim, histories.begin() + (i + 1) * dim, [](const History* hptr){ return hptr->empty(); })) {
		    // initialize actor
		    distr.draw_noncont(contexts.mutable_ctx().rng(), row);
		} else {
		    for (unsigned int k = 0; k < dim; ++k) {
			row[k] = histories[i * dim + k]->last_as_double(asof);
		    }
		}
            }
            distr.adjust_distribution(v);
            for (size_t i = 0; i < n; ++i) {
                const auto row = v.row(i);
                for (unsigned int k = 0; k < dim; ++k) {
                    const double x = row[k];
                    histories[i * dim + k]->append_or_correct(asof, x);
                }
            }
        }
        
        template class OperatorEnforcerMulti<Actor>;
        template class OperatorEnforcerMulti<Person>;
    }
}

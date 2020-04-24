#include "../common_features.hpp"
#include "mortality_enforcer.hpp"
#include "mortality.hpp"
#include "core/generic_distribution.hpp"
#include "core/statistics.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        MortalityEnforcer::MortalityEnforcer(std::shared_ptr<const Predicate<Person>> predicate, const std::vector<std::shared_ptr<const GenericDistribution<bool>>>& distributions)
            : Operator<Person>(true, FeatureUser<Feature>::feature_set_t({ CommonFeatures::MORTALITY() })), _predicate(predicate), _distributions(distributions) {
            if (!predicate) {
                throw std::domain_error("OperatorEnforcer: null predicate");
            }
            if (std::any_of(distributions.begin(), distributions.end(), [](const std::shared_ptr<const GenericDistribution<bool>>& distr) {
				return !distr;
                    })) {
                throw std::domain_error("OperatorEnforcer: one or more distributions is null");
            }
        }

        void MortalityEnforcer::apply(const std::vector<std::shared_ptr<Person>>& selected, const Contexts& contexts) const {
            if (contexts.immutable_ctx().schedule().nbr_dates() > _distributions.size()) {
                throw std::out_of_range("MortalityEnforcer: not enough distributions");
            }
            const Date asof = contexts.asof();
            const size_t idx = contexts.asof_idx();
            const GenericDistribution<bool>& distr = *(_distributions[idx]);
            const size_t n = selected.size();
            std::vector<double> v(n);
            for (size_t i = 0; i < n; ++i) {
				if (selected[i]->date_of_birth() > asof) {
					throw std::logic_error("MortalityEnforcer: unborn person in selected sample");
				}
                v[i] = selected[i]->is_alive(asof) ? Mortality::ALIVE : Mortality::DEAD;
            }
            Statistics::percentiles_inplace(v.begin(), v.end());
            for (size_t i = 0; i < n; ++i) {
                Person& person = *(selected[i]);
                const bool is_dead = distr.icdf_generic(v[i]);
				if (is_dead) {
					if (person.is_alive(asof)) {
						person.die(asof);
					}
				} else {
					if (!person.is_alive(asof)) { // minimal correction of death date
						person.die(asof + Period(PeriodType::DAYS, 1));
					}
				}                
            }
        }
    }
}

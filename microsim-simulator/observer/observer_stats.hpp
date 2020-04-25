#ifndef __AVERISERA_MS_OBSERVER_STATS_H
#define __AVERISERA_MS_OBSERVER_STATS_H

#include "../observer.hpp"
#include "observed_quantity.hpp"
#include "core/dates_fwd.hpp"
#include "core/running_statistics_multi.hpp"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace averisera {
    namespace microsim {
        template <class T> class Predicate;

        /** Observer which gathers statistics about particular type of members of the Population 
		@tparam V Type of collected values */
        template <class T, class V = double> class ObserverStats: public Observer {
        public:
            /*
              @param[in] variables Vector of observed variables (functions which take a reference to the member of Population and Contextx and return a value)
              @param[in] predicate Selects members to be included in the statistics
			  @param[in] calc_medians Calculate and save medians of observed variables 
              @param[in] precision Precision of outputs in digits
              @throw std::domain_error If any of the pointers is null
            */
            ObserverStats(std::shared_ptr<ObserverResultSaver> result_saver, const std::vector<ObservedQuantity<T>>& variables, std::shared_ptr<const Predicate<T>> predicate, bool calc_medians, unsigned int precision = 16);

            /** Observe variables with given names */
            ObserverStats(std::shared_ptr<ObserverResultSaver> result_saver, const std::vector<std::string>& variables, std::shared_ptr<const Predicate<T>> predicate, bool calc_medians, unsigned int precision = 16);

			/** Add another quantity */			
			ObserverStats<T>& add_variable(ObservedQuantity<T> qty) {
				_variables.push_back(qty);
				return *this;
			}

			/** Add another variable */
			ObserverStats<T>& add_variable(const std::string& variable) {
				return add_variable(ObservedQuantity<T>::last_as_double(variable));
			}
            
            void observe(const Population& population, const Contexts& ctx) override;

            void save_results(std::ostream& os, const ImmutableContext& im_ctx) const override;
        private:
			std::vector<ObservedQuantity<T>> _variables;
            std::shared_ptr<const Predicate<T>> _predicate;
            std::unordered_map<Date, RunningStatisticsMulti<V>> _stats;
			std::unordered_map<Date, std::vector<V>> medians_;
            unsigned int _precision;            
			bool calc_medians_;

			template <class Functor> void save_single_result(std::ostream& os, Functor f, const char* result_name, const std::vector<Date>& dates) const;

			void save_single_result(std::ostream& os, const std::unordered_map<Date, std::vector<V>>& results, const char* result_name, const std::vector<Date>& dates) const;
        };
    }
}

#endif // __AVERISERA_MS_OBSERVER_STATS_H

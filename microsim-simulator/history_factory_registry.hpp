// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MICROSIM_HISTORY_FACTORY_REGISTRY_HPP
#define __AVERISERA_MICROSIM_HISTORY_FACTORY_REGISTRY_HPP

#include "dispatcher.hpp"
#include "dispatcher_factory.hpp"
#include "history.hpp"
#include "history_factory.hpp"
#include "history_registry.hpp"
#include <stdexcept>
#include <vector>
#include <boost/format.hpp>

namespace averisera {
    namespace microsim {
        /** Registers History names/factories
          @tparam T Class using History
         */
		template <class T> class HistoryFactoryRegistry: public HistoryRegistry {
		public:
            /** History factory type */
            typedef HistoryFactory::factory_t factory_t;
            typedef Dispatcher<T, factory_t> dispatcher_t;
            typedef std::shared_ptr<const dispatcher_t> dispatcher_ptr_t;
            typedef size_t index_t;
            
            HistoryFactoryRegistry() = default;
            HistoryFactoryRegistry(const HistoryFactoryRegistry&) = default;
            HistoryFactoryRegistry(HistoryFactoryRegistry&& other) noexcept;
            
			/** Register a new variable
			@param[in] name Variable name
			@param[in] dispatcher Dispatches to factory function which creates History instance for this variable
			@return Index of this variable
			@throw std::domain_error If name is empty, name has been already registered or the dispatcher is null
			*/
			index_t register_variable(const std::string& name, dispatcher_ptr_t dispatcher) {
                if (!dispatcher) {
					LOG_ERROR() << "HistoryFactoryRegistry::register_variable: null dispatcher for variable " << name;
                    throw std::domain_error("HistoryFactoryRegistry: null dispatcher");
                }
                const index_t idx = HistoryRegistry::register_variable(name);
				variable_history_factory_dispatchers_.push_back(std::make_pair(name, dispatcher));
				LOG_DEBUG() << "HistoryFactoryRegistry::register_variable: registered dispatcher for variable " << name;
                return idx;
            }

            /** Register a new variable which is used for all objects with the same factory. 
              @param[in] name Variable name
              @param[in] factory Factory function 
              @return Index of this variable
              @throw std::domain_error If name has been already registered
			*/
            index_t register_variable(const std::string& name, factory_t factory) {
				if (!factory) {
					LOG_ERROR() << "HistoryFactoryRegistry::register_variable: null factor for variable " << name;
					throw std::domain_error("HistoryFactoryRegistry: null factory");
				}                
                const index_t idx = register_variable(name, DispatcherFactory::make_constant<T>(factory));     
                _common_factories[name] = factory;				
                return idx;
            }

			/** History factory dispatcher of idx-th variable  (used for testing)
              @throw std::out_of_range If idx > nbr_variables()
             */
			dispatcher_ptr_t variable_history_factory_dispatcher(index_t idx) const {
                if (idx >= nbr_variables()) {
                    throw std::out_of_range(boost::str(boost::format("HistoryFactoryRegistry: variable index %d out of range") % idx));
                }
				return variable_history_factory_dispatchers_[idx].second;
			}

            /** Create a vector of empty histories for this object, including histories with dispatchers and common histories.

              If the object is not selected by a dispatcher of some history, it will receive a null history.
              @param obj Object which needs the histories.              
             */
            std::vector<std::unique_ptr<History>> make_histories(const T& obj) const;

            /** Create a vector of empty histories using common factories (registered without a dispatcher) only.
            If some variable does not have a common factory, we will make a null history for it.            

			Used for testing.
            */
            std::vector<std::unique_ptr<History>> make_histories() const;
		private:
			std::vector<std::pair<std::string, dispatcher_ptr_t>> variable_history_factory_dispatchers_;
            std::unordered_map<std::string, factory_t> _common_factories; /**< Factories used to all objects */
		};

        template <class T> HistoryFactoryRegistry<T>::HistoryFactoryRegistry(HistoryFactoryRegistry&& other) noexcept
            : HistoryRegistry(std::move(other)),
			variable_history_factory_dispatchers_(std::move(other.variable_history_factory_dispatchers_)) {
        }
        
        template <class T> std::vector<std::unique_ptr<History>> HistoryFactoryRegistry<T>::make_histories(const T& obj) const {
            std::vector<std::unique_ptr<History>> hv;
            hv.reserve(variable_history_factory_dispatchers_.size());
            for (const auto& name_dispatcher_ptr_pair: variable_history_factory_dispatchers_) {
				const auto dispatcher_ptr = name_dispatcher_ptr_pair.second;
                if (dispatcher_ptr->predicate()->select_out_of_context(obj)) {
                    const auto factory = dispatcher_ptr->dispatch_out_of_context(obj);
                    hv.push_back(factory(name_dispatcher_ptr_pair.first));
                } else {
                    hv.push_back(nullptr);
                }
            }
            return std::move(hv);
        }

        template <class T> std::vector<std::unique_ptr<History>> HistoryFactoryRegistry<T>::make_histories() const {
            std::vector<std::unique_ptr<History>> hv;
            hv.reserve(variable_names().size());
            for (const std::string& name : variable_names()) {
                const auto iter = _common_factories.find(name);
                if (iter != _common_factories.end()) {
                    hv.push_back(iter->second(name));
                } else {
                    hv.push_back(nullptr);
                }
            }
            return std::move(hv);
        }
    }
}

#endif // __AVERISERA_MICROSIM_HISTORY_FACTORY_REGISTRY_HPP

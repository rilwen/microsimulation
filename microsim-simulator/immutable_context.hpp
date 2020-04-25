/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_IMMUTABLE_CONTEXT_H
#define __AVERISERA_MS_IMMUTABLE_CONTEXT_H

#include "history_factory_registry.hpp"
#include "microsim-core/ethnicity.hpp"
#include "microsim-core/schedule.hpp"
#include <map>
#include <memory>

namespace averisera {
    namespace microsim {
	    class History;
        class Person;
        template <class T> class Predicate;

	    /** @brief Encapsulates shared data which does not change during the simulation.
	     */
	    class ImmutableContext {
	    public:
            // Convenience typedef
            typedef HistoryFactoryRegistry<Person>::dispatcher_ptr_t person_history_dispatcher_ptr_t;
            typedef size_t varidx_t; /**< Type used to index variables */
            
            /** Default context */
            ImmutableContext() : ImmutableContext(Schedule(), Ethnicity::IndexConversions()) {}
            
	        /**
	          @param[in] schedule Simulation schedule
			  @param[in] ic Ethnicity index conversions
	        */
	        ImmutableContext(const Schedule& schedule, const Ethnicity::IndexConversions& ic = Ethnicity::IndexConversions());

			/**
			@param[in] schedule Simulation schedule
			@param[in] ic Ethnicity index conversions
			*/
			ImmutableContext(Schedule&& schedule, Ethnicity::IndexConversions&& ic = Ethnicity::IndexConversions());

            ImmutableContext(const ImmutableContext&) = default;

            ImmutableContext(ImmutableContext&& other) noexcept;

	        /** HistoryFactoryRegistry for Person class */
            const HistoryFactoryRegistry<Person>& person_history_registry() const {
		        return _person_data.history_factory_registry;
	        }

	        const Schedule& schedule() const {
		        return _schedule;
	        }

            /** Register a variable for Person class
              @param[in] name History variable name
              @param[in] dispatcher Dispatcher to factories used to initialise histories of this variable
            @see HistoryFactoryRegistry
            */
            varidx_t register_person_variable(const std::string& name, person_history_dispatcher_ptr_t dispatcher);

            /** Register a variable for Person class
              @param[in] name History variable name
              @param[in] factory History factory to be used for all Person objects
            @see HistoryFactoryRegistry
            */
            varidx_t register_person_variable(const std::string& name, HistoryFactory::factory_t factory);

			const Ethnicity::IndexConversions& ethnicity_conversions() const {
				return ethnic_conv_;
			}

			template <class T> void collect_history_requirements(std::vector<std::shared_ptr<Operator<T>>>& operators);
	    private:
            /** Data for a class of Actor */
            template <class T> struct ActorCtxData {
				ActorCtxData() = default;
				ActorCtxData(const ActorCtxData<T>&) = default;
				ActorCtxData(ActorCtxData<T>&& other) noexcept
                    : history_factory_registry(std::move(other.history_factory_registry)) {
                }                
                HistoryFactoryRegistry<T> history_factory_registry;
            };
			ActorCtxData<Person> _person_data;
	        Schedule _schedule;            
			Ethnicity::IndexConversions ethnic_conv_; /** Conversion methods for ethnic group indices */
	    };
    }
}

#endif 

/*
 * (C) Averisera Ltd 2015
 */
#include "immutable_context.hpp"

namespace averisera {
    namespace microsim {
	    ImmutableContext::ImmutableContext(const Schedule& schedule, const Ethnicity::IndexConversions& ic)
	    : _schedule(schedule), ethnic_conv_(ic) {
	    }

		ImmutableContext::ImmutableContext(Schedule&& schedule, Ethnicity::IndexConversions&& ic)
			: _schedule(std::move(schedule)), ethnic_conv_(std::move(ic)) {
		}

        ImmutableContext::ImmutableContext(ImmutableContext&& other) noexcept
        : _person_data(std::move(other._person_data)), _schedule(std::move(other._schedule)), ethnic_conv_(std::move(other.ethnic_conv_)) {
        }

        ImmutableContext::varidx_t ImmutableContext::register_person_variable(const std::string& name, ImmutableContext::person_history_dispatcher_ptr_t dispatcher) {
            return _person_data.history_factory_registry.register_variable(name, dispatcher);            
        }

        ImmutableContext::varidx_t ImmutableContext::register_person_variable(const std::string& name, HistoryFactory::factory_t factory) {
            return _person_data.history_factory_registry.register_variable(name, factory);            
        }

		// to avoid C4503 warning in Visual Studio C++
		template <class T> struct reqmap_value : public std::pair<std::vector<typename HistoryFactoryRegistry<T>::factory_t>, std::vector<std::shared_ptr<const Predicate<T> > > > {
		};

		template <class T> struct reqmap { 
			typedef reqmap_value<T> value_type;
			typedef std::unordered_map<std::string, value_type> t;
		};

		template <class T> struct user_reqmap {
			//typedef std::vector<std::shared_ptr<const Predicate<T> > > value_type;
			typedef std::vector<std::string> t;
		};

		template <class T> typename reqmap<T>::t build_req_map(const std::vector<std::shared_ptr<Operator<T> > >& operators) {
			typename reqmap<T>::t map;
			for (const auto& ptr : operators) {
				for (const auto& req : ptr->requirements()) {
					const std::string& name = std::get<0>(req);
					typename reqmap<T>::value_type& pair = map[name];
					pair.first.push_back(std::get<1>(req));
					pair.second.push_back(std::get<2>(req));
				}
			}
			return map;
		}

		template <class T> typename user_reqmap<T>::t build_user_req_map(const std::vector<std::shared_ptr<Operator<T> > >& operators) {
			typename user_reqmap<T>::t map;
			for (const auto& ptr : operators) {
				for (const auto& name : ptr->user_requirements()) {
					map.push_back(name);
				}
			}
			return map;
		}

		template <class T> void ImmutableContext::collect_history_requirements(std::vector<std::shared_ptr<Operator<T>>>& operators) {
			const typename reqmap<T>::t map(build_req_map(operators));
			for (auto kv : map) {
				const auto& v = kv.second;
				const std::shared_ptr<const Dispatcher<T, typename HistoryFactoryRegistry<T>::factory_t> >  dispatcher = DispatcherFactory::make_group(v.second, v.first, Feature::empty());
				register_person_variable(kv.first, dispatcher);
			}
			const typename user_reqmap<T>::t user_map(build_user_req_map(operators));
			for (const auto& name : user_map) {
				if (map.find(name) == map.end()) {
					throw std::logic_error(boost::str(boost::format("ImmutableContext: no HistoryGenerator for variable %s") % name));
				}
			}
		}

		template void ImmutableContext::collect_history_requirements<Person>(std::vector<std::shared_ptr<Operator<Person>>>& operators);
    }
}

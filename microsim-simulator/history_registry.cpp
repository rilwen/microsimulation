/*
* (C) Averisera Ltd 2015
*/
#include "history_registry.hpp"
#include <boost/format.hpp>

namespace averisera {
	namespace microsim {
        HistoryRegistry::~HistoryRegistry() {
        }

        HistoryRegistry::HistoryRegistry(HistoryRegistry&& other) noexcept
            : _variables_by_name(std::move(other._variables_by_name)),
              _variable_names(std::move(other._variable_names)) {
        }
        
		HistoryRegistry::index_t HistoryRegistry::register_variable(const std::string& name) {
            if (name.empty()) {
                throw std::domain_error("HistoryRegistry: empty name");
            }
            const auto name_it = _variables_by_name.find(name);
			if (name_it != _variables_by_name.end()) {
                throw std::domain_error(boost::str(boost::format("HistoryRegistry: variable name %s already registered at index %d") % name % name_it->second));
			}
			const index_t idx = _variable_names.size();
			_variable_names.push_back(name);
			_variables_by_name[name] = idx;
			return idx;
		}

		HistoryRegistry::index_t HistoryRegistry::variable_index(const std::string& name) const {
			const auto iter = _variables_by_name.find(name);
			if (iter != _variables_by_name.end()) {
				return iter->second;
			} else {
				throw std::domain_error(boost::str(boost::format("HistoryRegistry: no such variable %s") % name));
			}
		}

	}
}

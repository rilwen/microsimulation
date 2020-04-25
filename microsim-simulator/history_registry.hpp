/*
* (C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_MS_HISTORY_REGISTRY_H
#define __AVERISERA_MS_HISTORY_REGISTRY_H

#include <cassert>
#include <unordered_map>
#include <string>
#include <vector>

namespace averisera {
	namespace microsim {
		/** Registers History names for an Actor implementation. 
         */
		class HistoryRegistry {
		public:
            typedef size_t index_t;
            
            HistoryRegistry() = default;
            virtual ~HistoryRegistry();
            HistoryRegistry(const HistoryRegistry&) = default;
            HistoryRegistry(HistoryRegistry&& other) noexcept;
            
			/** Register a new variable
			@param[in] name Variable name
			@return Index of this variable
			@throw std::domain_error If name has been already registered or is empty
			*/
			index_t register_variable(const std::string& name);

			/** Number of registered variables */
			index_t nbr_variables() const {
				return _variable_names.size();
			}

			/** Name of idx-th variable */
			const std::string& variable_name(index_t idx) const {
				assert(idx < nbr_variables());
				return _variable_names[idx];
			}

			/** Is the variable of this name registered? */
			bool has_variable(const std::string& name) const {
				return _variables_by_name.find(name) != _variables_by_name.end();
			}

			/** Index of variable with this name (if it is registered).
			@param[in] name Variable name
			@throw std::domain_error If not registered.
			*/
			index_t variable_index(const std::string& name) const;

			typedef std::unordered_map<std::string, index_t>::const_iterator const_iterator;

			const_iterator begin() const {
				return _variables_by_name.begin();
			}

			const_iterator end() const {
				return _variables_by_name.end();
			}
        protected:
            const std::vector<std::string>& variable_names() const {
                return _variable_names;
            }
		private:
			std::unordered_map<std::string, index_t> _variables_by_name;
			std::vector<std::string> _variable_names;
		};
	}
}

#endif // __AVERISERA_MS_HISTORY_REGISTRY_H

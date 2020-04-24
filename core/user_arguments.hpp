/*
(C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_USER_ARGUMENTS_H
#define __AVERISERA_USER_ARGUMENTS_H

#include <iosfwd>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/format.hpp>
#include "stl_utils.hpp"
#include "utils.hpp"

namespace averisera {
	/// Handling and decoding user arguments
	class UserArguments {
	public:
		/// Split a comma-separated list into vector of elements
		static void split_list(const char* csl, std::vector<std::string>& elems);

		/// Split a "key=value" string into key and value and remove leading and trailing whitespaces from them.
		static void get_key_value(const char* line, std::string& key, std::string& value);

		/// Get a (key, value) map from a text stream
		static std::unordered_map<std::string, std::string> get_keys_values(std::istream& input);

		/** Load user arguments from file or standard input (if fname is nullptr)
		@throw std::runtime_error If fname != nullptr but files does not exist or cannot be opened.
		*/
		UserArguments(const char* fname);

		/// Load user arguments from input stream
		UserArguments(std::istream& input);

		UserArguments(const UserArguments&) = delete;

		UserArguments& operator=(const UserArguments&) = delete;

		// key: non-empty string
		// value: default value
		template <class T> T get(const std::string& key, T value) const {
			if (!key.empty()) {
				const auto it = _keys_values.find(key);
				if (it != _keys_values.end()) {
                    try {
                        value = boost::lexical_cast<T>(it->second);
                    } catch (std::exception& e) {
                        std::stringstream ss;
                        ss << "Error getting value for key '" << key << "' from source '" << it->second << "': " << e.what();
                        throw std::runtime_error(ss.str());
                    }
				}
			} else {
				throw std::domain_error("Empty key");
			}
			record_key_value(key, value);
			return value;
		}

		// key: non-empty string
		template <class T> T get(const std::string& key) const {
			T value;
			if (!key.empty()) {
				const auto it = _keys_values.find(key);
				if (it != _keys_values.end()) {
					try {
						value = Utils::from_string<T>(it->second);                        
                    } catch (std::exception& e) {
						throw std::runtime_error(boost::str(boost::format("UserArguments: Error getting value for key %s from source %s: %s") % key % it->second % e.what()));
                    }
				} else {
					throw report_no_value(key);
				}
			} else {
				throw std::domain_error("Empty key");
			}
			record_key_value(key, value);
			return value;
		}

		/// Get a comma-separated list of values for given key.
		template <class T> void get(const std::string& key, std::vector<T>& values, bool mandatory = false) const {
			if (contains(key)) {
				const std::string csl(get<std::string>(key));
				std::vector<std::string> tokens;
				split_list(csl.c_str(), tokens);
				if (tokens.size() == 1 && tokens.front() == "") {
					tokens.clear();
				}
				values.resize(tokens.size());
				std::transform(tokens.begin(), tokens.end(), values.begin(), [](const std::string& elem){return boost::lexical_cast<T>(elem); });
			} else {
				if (mandatory) {
					report_no_value(key);
				} else {
					values.clear();
				}
			}
			record_key_value(key, values);
		}

		// Did the user provide the value for this key?
		// key: non-empty string
		bool contains(const std::string& key) const;

		size_t size() const {
			return _keys_values.size();
		}

		/// Print all provided argument.
		void print(std::ostream& out) const;

		/// Return a map of (key, value) pairs returned by "get" functions.
		const std::map<std::string, std::string>& read_keys_values() const {
			return *read_keys_values_;
		}
	private:
		std::unordered_map<std::string, std::string> _keys_values;

		/// (key, value) pairs returned by get functions.
		std::unique_ptr<std::map<std::string, std::string>> read_keys_values_;

		static std::runtime_error report_no_value(const std::string& key);

		/// Record (key, value) pair before returning it from "get" function.
		/// We do this to keep track of all settings read by the application.
		template <class T> void record_key_value(const std::string& key, const T& value) const {
			std::stringstream ss;
			ss << value;
			(*read_keys_values_)[key] = ss.str();
		}
	};
}

#endif

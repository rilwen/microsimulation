/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include "user_arguments.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

namespace averisera {
	// Local methods
	namespace {
		static const std::string _whitespaces = " \t\r\n";

		static std::string trim(const std::string& str)
		{
			const auto strBegin = str.find_first_not_of(_whitespaces);
			if (strBegin == std::string::npos)
				return "";

			const auto strEnd = str.find_last_not_of(_whitespaces);
			const auto strRange = strEnd - strBegin + 1;

			return str.substr(strBegin, strRange);
		}

		static std::string unquote(const std::string& str) {
			if (str.size() > 1 && str[0] == '"' && str[str.size() - 1] == '"') {
				return str.substr(1, str.size() - 2);
			} else {
				return str;
			}
		}
	}

	void UserArguments::split_list(const char* csl, std::vector<std::string>& elems) {
		boost::split(elems, csl, boost::is_any_of(","));
	}

	void UserArguments::get_key_value(const char* line, std::string& key, std::string& value) {
		std::vector<std::string> tokens;
		boost::split(tokens, line, boost::is_any_of("="));
		key = "";
		value = "";
		const size_t n_tok = tokens.size();
		if (n_tok) {
			key = trim(tokens[0]);
		}
		if (n_tok > 1) {
			value = unquote(trim(tokens[1]));
		}
	}

	std::unordered_map<std::string, std::string> UserArguments::get_keys_values(std::istream& input) {
		std::string line;
		std::unordered_map<std::string, std::string> map;
		while (std::getline(input, line)) {
			line = trim(line);
			if (!line.empty() && line[0] != '#') {
				std::string key;
				std::string value;
				get_key_value(line.c_str(), key, value);
				if (key != "") {
					map[key] = value;
				} else {
					throw std::runtime_error("Empty key");
				}
			}
		}
		return map;
	}

	UserArguments::UserArguments(const char* fname) 
		: read_keys_values_(new std::map<std::string, std::string>())
	{
		if (fname != nullptr) {
			std::ifstream file(fname);
			if (!file.is_open()) {
				throw std::runtime_error(boost::str(boost::format("UserArguments: cannot open file %s") % fname));
			}
			_keys_values = get_keys_values(file);
		} else {
			_keys_values = get_keys_values(std::cin);
		}
	}

	UserArguments::UserArguments(std::istream& input) 
		: _keys_values(get_keys_values(input)),
		read_keys_values_(new std::map<std::string, std::string>())
	{
	}

	void UserArguments::print(std::ostream& out) const {
		const auto end = _keys_values.end();
		for (auto it = _keys_values.begin(); it != end; ++it) {
			out << it->first << " -> " << it->second << std::endl;
		}
	}

	bool UserArguments::contains(const std::string& key) const {
		if (!key.empty()) {
			return _keys_values.find(key) != _keys_values.end();
		} else {
			throw std::domain_error("Empty key");
		}
	}

	std::runtime_error UserArguments::report_no_value(const std::string& key) {
		std::stringstream ss;
		ss << "No value for key: " << key;
		return std::runtime_error(ss.str());
	}
}


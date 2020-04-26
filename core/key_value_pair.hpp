// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_KEY_VALUE_PAIR_H
#define __AVERISERA_KEY_VALUE_PAIR_H

#include <string>
#include "object_value.hpp"

namespace averisera {
	// (Key, value) pair for generic property lists.
	struct KeyValuePair {
		KeyValuePair() = default;		
		KeyValuePair(const KeyValuePair&) = default;
		KeyValuePair& operator=(const KeyValuePair&) = default;
		KeyValuePair(const std::string& n_key, const ObjectValue& n_value);
		KeyValuePair(std::string&& n_key, ObjectValue&& n_value);

		std::string key;
		ObjectValue value;
	};
}

#endif
#include "key_value_pair.hpp"

namespace averisera {
	KeyValuePair::KeyValuePair(const std::string& n_key, const ObjectValue& n_value)
		: key(n_key), value(n_value)
	{}

	KeyValuePair::KeyValuePair(std::string&& n_key, ObjectValue&& n_value)
		: key(n_key), value(n_value)
	{}
}
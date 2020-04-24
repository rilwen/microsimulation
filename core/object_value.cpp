#include "object_value.hpp"
#include <algorithm>
#include <ostream>
#include <stdexcept>

namespace averisera {
	ObjectValue::ObjectValue(double x) {
		init(x, Type::DOUBLE);
	}

	ObjectValue::ObjectValue(int i) {
		init(i, Type::INT);
	}

	ObjectValue::ObjectValue(bool b) {
		init(b, Type::BOOL);
	}

	ObjectValue::ObjectValue(const std::string& s) {
		init(s, Type::STRING);
	}

	ObjectValue::ObjectValue(const char* s) {
		init(std::string(s), Type::STRING);
	}

	ObjectValue::ObjectValue(const std::vector<ObjectValue>& vector) {
		init(vector, Type::VECTOR);
	}

	ObjectValue ObjectValue::from_object(std::shared_ptr<void> o) {
		ObjectValue ov;
		ov.init(o, Type::OBJECT);
		return ov;
	}

	template <class T> void ObjectValue::init(const T& value, Type type) {
		_type = type;
		_value = new T(value);
	}

	template <class T> void ObjectValue::init_move(T&& value, Type type) {
		_value = new T;
		*_value = std::move(value);
		_type = type;
	}

	ObjectValue::ObjectValue() 
		: _value(nullptr), _type(Type::NONE)
	{}

	ObjectValue::ObjectValue(ObjectValue&& other) 
		: _value(other._value), _type(other._type)
	{
		other._value = nullptr;
		other._type = Type::NONE;
	}

	ObjectValue::ObjectValue(const ObjectValue& other) 
	{
		switch (other._type) {
		case Type::BOOL:
			init(other.as_bool(), other._type);
			break;
		case Type::DOUBLE:
			init(other.as_double(), other._type);
			break;
		case Type::INT:
			init(other.as_int(), other._type);
			break;
		case Type::STRING:
			init(other.as_string(), other._type);
			break;
		case Type::OBJECT:
			init(*static_cast<std::shared_ptr<void>*>(other._value), other._type);
            break;
		case Type::VECTOR:
			init(other.as_vector(), other._type);
            break;
		case Type::NONE:
			_value = nullptr;
			_type = Type::NONE;
			break;
		default:
			throw std::logic_error("Unknown Type value");
		}
	}

	ObjectValue& ObjectValue::operator=(const ObjectValue& other) {
		if (this != &other) {
			ObjectValue clone(other);
			swap(*this, clone);
		}
		return *this;
	}

    std::vector<double> ObjectValue::as_double_vector() const {
        assert(_type == Type::VECTOR);
        const auto& vo = as_vector();
        std::vector<double> vd(vo.size());
        std::transform(vo.begin(), vo.end(), vd.begin(), [](const ObjectValue& o) { return o.as_double();  });
        return vd;
    }

	ObjectValue::~ObjectValue() {
		if (_value != nullptr) {
			assert(_type != Type::NONE);
			switch (_type) {
				case Type::BOOL:
					delete static_cast<bool*>(_value);
					break;
				case Type::DOUBLE:
					delete static_cast<double*>(_value);
					break;
				case Type::INT:
					delete static_cast<int*>(_value);
					break;
				case Type::STRING:
					delete static_cast<std::string*>(_value);
					break;
				case Type::OBJECT:
					delete static_cast<std::shared_ptr<void>*>(_value);
					break;
				case Type::VECTOR:
					delete static_cast<std::vector<ObjectValue>*>(_value);
					break;
				default:
					assert(false);
			}
		}
	}

	std::ostream& operator<<(std::ostream& os, const ObjectValue::Type typ) {
		if (typ == ObjectValue::Type::DOUBLE) {
			os << "DOUBLE";
		} else if (typ == ObjectValue::Type::INT) {
			os << "INT";
		} else if (typ == ObjectValue::Type::BOOL) {
			os << "BOOL";
		} else if (typ == ObjectValue::Type::STRING) {
			os << "STRING";
		} else if (typ == ObjectValue::Type::OBJECT) {
			os << "OBJECT";
		} else if (typ == ObjectValue::Type::VECTOR) {
			os << "VECTOR";
		} else if (typ == ObjectValue::Type::NONE) {
			os << "NONE";
		} else {
			throw std::invalid_argument("Conversion of this ObjectValue::Type not implemented");
		}
		return os;
	}
}

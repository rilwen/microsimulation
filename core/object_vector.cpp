#include "object_vector.hpp"
#include <cassert>
#include <stdexcept>
#include <boost/format.hpp>
#include "core/printable.hpp"
#include "stl_utils.hpp"

namespace averisera {

	
	template <> ObjectVector::Type ObjectVector::get_type<double>() {
		return ObjectVector::Type::DOUBLE;
	}

	template <> ObjectVector::Type ObjectVector::get_type<float>() {
		return ObjectVector::Type::FLOAT;
	}

	template <> ObjectVector::Type ObjectVector::get_type<int8_t>() {
		return ObjectVector::Type::INT8;
	}

	template <> ObjectVector::Type ObjectVector::get_type<int16_t>() {
		return ObjectVector::Type::INT16;
	}

	template <> ObjectVector::Type ObjectVector::get_type<int32_t>() {
		return ObjectVector::Type::INT32;
	}

	template <> ObjectVector::Type ObjectVector::get_type<uint8_t>() {
		return ObjectVector::Type::UINT8;
	}

	template <> ObjectVector::Type ObjectVector::get_type<uint16_t>() {
		return ObjectVector::Type::UINT16;
	}

	template <> ObjectVector::Type ObjectVector::get_type<uint32_t>() {
		return ObjectVector::Type::UINT32;
	}

    ObjectVector::Type ObjectVector::type_from_string(const std::string& str) {
        if (str == "none") {
            return ObjectVector::Type::NONE;
        } else if (str == print_type_name<double>()) {
            return get_type<double>();
		} else if (str == print_type_name<float>()) {
			return get_type<float>();
        } else if (str == print_type_name<int8_t>()) {
            return get_type<int8_t>();
		} else if (str == print_type_name<int16_t>()) {
			return get_type<int16_t>();
		} else if (str == print_type_name<int32_t>()) {
            return get_type<int32_t>();
        } else if (str == print_type_name<uint8_t>()) {
            return get_type<uint8_t>();
		} else if (str == print_type_name<uint16_t>()) {
			return get_type<uint16_t>();
		} else if (str == print_type_name<uint32_t>()) {
			return get_type<uint32_t>();
		} else {
            throw std::domain_error(boost::str(boost::format("ObjectVector: unknown type name: %s") % str));
        }
    }

	ObjectVector::ObjectVector()
		: _type(Type::NONE) {}

	ObjectVector::ObjectVector(Type type) {
		switch (type) {
		case Type::DOUBLE:
			init(std::vector<double>(), type);
			break;
		case Type::FLOAT:
			init(std::vector<float>(), type);
			break;
		case Type::INT8:
			init(std::vector<int8_t>(), type);
			break;
		case Type::INT16:
			init(std::vector<int16_t>(), type);
			break;
		case Type::INT32:
			init(std::vector<int32_t>(), type);
			break;
		case Type::UINT8:
			init(std::vector<uint8_t>(), type);
			break;
		case Type::UINT16:
			init(std::vector<uint16_t>(), type);
			break;
		case Type::UINT32:
			init(std::vector<uint32_t>(), type);
			break;
		case Type::NONE:
			_type = type;
			break;
		default:
			throw std::domain_error(boost::str(boost::format("ObjectVector: unknown type %d") % static_cast<int>(type)));
		}
	}

	ObjectVector::ObjectVector(const ObjectVector& other) {
		switch (other._type) {
		case Type::DOUBLE:
			init(other.as<double>(), other._type);
			break;
		case Type::FLOAT:
			init(other.as<float>(), other._type);
			break;
		case Type::INT8:
			init(other.as<int8_t>(), other._type);
			break;
		case Type::INT16:
			init(other.as<int16_t>(), other._type);
			break;
		case Type::INT32:
			init(other.as<int32_t>(), other._type);
			break;
		case Type::UINT8:
			init(other.as<uint8_t>(), other._type);
			break;
		case Type::UINT16:
			init(other.as<uint16_t>(), other._type);
			break;
		case Type::UINT32:
			init(other.as<uint32_t>(), other._type);
			break;
		case Type::NONE:
			_type = other._type;
			break;
		default:
			throw std::domain_error(boost::str(boost::format("ObjectVector: unknown type %d") % static_cast<int>(other._type)));
		}
	}

	ObjectVector::ObjectVector(ObjectVector&& other) noexcept {
		_type = other._type;
		_value = other._value;
		other._value = nullptr;
		other._type = Type::NONE;
	}

	void ObjectVector::swap(ObjectVector& other) noexcept {
		std::swap(_type, other._type);
		std::swap(_value, other._value);
	}

	ObjectVector& ObjectVector::operator=(ObjectVector&& other) noexcept {
		if (this != &other) {
			ObjectVector copy(std::move(other));
			this->swap(copy);
		}
		return *this;
	}

	template <class V> ObjectVector::ObjectVector(const std::vector<V>& v)
		: _value(new std::vector<V>(v)), _type(get_type<V>()) {		
	}
	template ObjectVector::ObjectVector(const std::vector<double>& v);
	template ObjectVector::ObjectVector(const std::vector<float>& v);
	template ObjectVector::ObjectVector(const std::vector<int8_t>& v);
	template ObjectVector::ObjectVector(const std::vector<int16_t>& v);
	template ObjectVector::ObjectVector(const std::vector<int32_t>& v);
	template ObjectVector::ObjectVector(const std::vector<uint8_t>& v);	
	template ObjectVector::ObjectVector(const std::vector<uint16_t>& v);
	template ObjectVector::ObjectVector(const std::vector<uint32_t>& v);

	template <class V> ObjectVector::ObjectVector(std::vector<V>&& v)
		: _value(new std::vector<V>()), _type(get_type<V>()) {
		as<V>() = std::move(v);
	}
	template ObjectVector::ObjectVector(std::vector<double>&& v);
	template ObjectVector::ObjectVector(std::vector<float>&& v);
	template ObjectVector::ObjectVector(std::vector<int8_t>&& v);
	template ObjectVector::ObjectVector(std::vector<int16_t>&& v);
	template ObjectVector::ObjectVector(std::vector<int32_t>&& v);
	template ObjectVector::ObjectVector(std::vector<uint8_t>&& v);
	template ObjectVector::ObjectVector(std::vector<uint16_t>&& v);
	template ObjectVector::ObjectVector(std::vector<uint32_t>&& v);
	
	template <class T> void ObjectVector::init(const std::vector<T>& value, Type type) {
		_value = new std::vector<T>(value);
		_type = type;
	}

	size_t ObjectVector::size() const {
		switch (_type) {
		case Type::DOUBLE:
			return as<double>().size();
		case Type::FLOAT:
			return as<float>().size();
		case Type::INT8:
			return as<int8_t>().size();
		case Type::INT16:
			return as<int16_t>().size();
		case Type::INT32:
			return as<int32_t>().size();
		case Type::UINT8:
			return as<uint8_t>().size();
		case Type::UINT16:
			return as<uint16_t>().size();
		case Type::UINT32:
			return as<uint32_t>().size();
		case Type::NONE:
			return 0;
		default:
			throw std::domain_error(boost::str(boost::format("ObjectVector: unknown type %d") % static_cast<int>(_type)));
		}
	}

	ObjectVector::~ObjectVector() {
		if (_type != Type::NONE) {
			switch (_type) {
			case Type::DOUBLE:
				delete static_cast<std::vector<double>*>(_value);
				break;
			case Type::FLOAT:
				delete static_cast<std::vector<float>*>(_value);
				break;
			case Type::INT8:
				delete static_cast<std::vector<int8_t>*>(_value);
				break;
			case Type::INT16:
				delete static_cast<std::vector<int16_t>*>(_value);
				break;
			case Type::INT32:
				delete static_cast<std::vector<int32_t>*>(_value);
				break;
			case Type::UINT8:
				delete static_cast<std::vector<uint8_t>*>(_value);
				break;
			case Type::UINT16:
				delete static_cast<std::vector<uint16_t>*>(_value);
				break;
			case Type::UINT32:
				delete static_cast<std::vector<uint32_t>*>(_value);
				break;
			default:
				assert(false && "Attempt to destroy unknown ObjectVector type");
			}
		}
	}

	template <class T> std::vector<T>& ObjectVector::as() {
        assert(_type == get_type<T>());
        const auto ptr = static_cast<std::vector<T>*>(_value);
        assert(ptr);
		return *ptr;
	}

	template <class T> const std::vector<T>& ObjectVector::as() const {
        assert(_type == get_type<T>());
        const auto ptr = static_cast<const std::vector<T>*>(_value);
        assert(ptr);
		return *ptr;
	}

	template std::vector<double>& ObjectVector::as<double>();
	template std::vector<float>& ObjectVector::as<float>();
	template std::vector<int8_t>& ObjectVector::as<int8_t>();
	template std::vector<int16_t>& ObjectVector::as<int16_t>();
	template std::vector<int32_t>& ObjectVector::as<int32_t>();
	template std::vector<uint8_t>& ObjectVector::as<uint8_t>();
	template std::vector<uint16_t>& ObjectVector::as<uint16_t>();
	template std::vector<uint32_t>& ObjectVector::as<uint32_t>();
	template const std::vector<double>& ObjectVector::as<double>() const;
	template const std::vector<float>& ObjectVector::as<float>() const;
	template const std::vector<int8_t>& ObjectVector::as<int8_t>() const;
	template const std::vector<int16_t>& ObjectVector::as<int16_t>() const;
	template const std::vector<int32_t>& ObjectVector::as<int32_t>() const;
	template const std::vector<uint8_t>& ObjectVector::as<uint8_t>() const;
	template const std::vector<uint16_t>& ObjectVector::as<uint16_t>() const;
	template const std::vector<uint32_t>& ObjectVector::as<uint32_t>() const;

	
	void ObjectVector::reserve(size_t capacity) {
		switch (_type) {
		case Type::DOUBLE:
			as<double>().reserve(capacity);
			break;
		case Type::FLOAT:
			as<float>().reserve(capacity);
			break;
		case Type::INT8:
			as<int8_t>().reserve(capacity);
			break;
		case Type::INT16:
			as<int16_t>().reserve(capacity);
			break;
		case Type::INT32:
			as<int32_t>().reserve(capacity);
			break;
		case Type::UINT8:
			as<uint8_t>().reserve(capacity);
			break;
		case Type::UINT16:
			as<uint16_t>().reserve(capacity);
			break;
		case Type::UINT32:
			as<uint32_t>().reserve(capacity);
			break;
		case Type::NONE:
			throw std::domain_error("ObjectVector: reserving size in null");
		default:
			throw std::domain_error(boost::str(boost::format("ObjectVector: unknown type %d") % static_cast<int>(_type)));
		}
	}

	void ObjectVector::resize(size_t capacity) {
		switch (_type) {
		case Type::DOUBLE:
			as<double>().resize(capacity);
			break;
		case Type::FLOAT:
			as<float>().resize(capacity);
			break;
		case Type::INT8:
			as<int8_t>().resize(capacity);
			break;
		case Type::INT16:
			as<int16_t>().resize(capacity);
			break;
		case Type::INT32:
			as<int32_t>().resize(capacity);
			break;
		case Type::UINT8:
			as<uint8_t>().resize(capacity);
			break;
		case Type::UINT16:
			as<uint16_t>().resize(capacity);
			break;
		case Type::UINT32:
			as<uint32_t>().resize(capacity);
			break;
		case Type::NONE:
			throw std::domain_error("ObjectVector: resizing null");
		default:
			throw std::domain_error(boost::str(boost::format("ObjectVector: unknown type %d") % static_cast<int>(_type)));
		}
	}

    std::ostream& operator<<(std::ostream& os, ObjectVector::Type typ) {
        switch (typ) {
        case ObjectVector::Type::DOUBLE:
            os << print_type_name<double>();
            break;
		case ObjectVector::Type::FLOAT:
			os << print_type_name<float>();
			break;
		case ObjectVector::Type::INT8:
            os << print_type_name<int8_t>();
            break;
		case ObjectVector::Type::INT16:
			os << print_type_name<int16_t>();
			break;
		case ObjectVector::Type::INT32:
            os << print_type_name<int32_t>();
            break;
        case ObjectVector::Type::UINT8:
            os << print_type_name<uint8_t>();
            break;
		case ObjectVector::Type::UINT16:
			os << print_type_name<uint16_t>();
			break;
		case ObjectVector::Type::UINT32:
			os << print_type_name<uint32_t>();
			break;
		case ObjectVector::Type::NONE:
            os << "none";
            break;
        default:
			throw std::domain_error(boost::str(boost::format("ObjectVector: unknown type %d") % static_cast<int>(typ)));
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const ObjectVector& v) {
        os << v.type();		
        switch (v.type()) {
        case ObjectVector::Type::DOUBLE:
            os << "|" << v.as<double>();
            break;
		case ObjectVector::Type::FLOAT:
			os << "|" << v.as<float>();
			break;
		case ObjectVector::Type::INT8:
            os << "|" << v.as<int8_t>();
            break;
		case ObjectVector::Type::INT16:
			os << "|" << v.as<int16_t>();
			break;
		case ObjectVector::Type::INT32:
            os << "|" << v.as<int32_t>();
            break;
        case ObjectVector::Type::UINT8:
            os << "|" << v.as<uint8_t>();
            break;
		case ObjectVector::Type::UINT16:
			os << "|" << v.as<uint16_t>();
			break;
		case ObjectVector::Type::UINT32:
			os << "|" << v.as<uint32_t>();
			break;
		case ObjectVector::Type::NONE:
            break;
        default:
			throw std::domain_error(boost::str(boost::format("ObjectVector: unknown type %d") % static_cast<int>(v.type())));			
        }
        return os;
    }
}

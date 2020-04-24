#pragma once
#include <cstdint>
#include <cstdlib>
#include <iosfwd>
#include <string>
#include <vector>
#include "core/math_utils.hpp"

namespace averisera {
	/** Vector version of ObjectValue. All elements stored are of the same C++ type. 

      TODO: use parametric lambdas to make the code shorter
     */
	class ObjectVector {
	public:
		/** Value type */
		enum class Type : int8_t {
			DOUBLE, /**< double */
			FLOAT, /**< float */
			INT8, /**< int8_t */
			INT16, /**< int16_t */
			INT32, /**< int32_t */
			UINT8, /**< uint8_t */
			UINT16, /**< uint16_t */
			UINT32, /**< int32_t */
			NONE /** null */
		};

        /** Convert string to Type */
        static Type type_from_string(const std::string& str);

		/** Null vector */
		ObjectVector();

		ObjectVector(const ObjectVector& other);

		ObjectVector(ObjectVector&& other) noexcept;

		ObjectVector& operator=(ObjectVector&& other) noexcept;

		void swap(ObjectVector& other) noexcept;

		/** Empty vector */
		ObjectVector(Type type);

		/** Copy constructor
		@tparam V one of supported value types */
		template <class V> explicit ObjectVector(const std::vector<V>& v);

		/** Move constructor
		@tparam V one of supported value types */
		template <class V> ObjectVector(std::vector<V>&& v);

		~ObjectVector();

		/** Length of the vector */
		size_t size() const;

		bool is_null() const {
			return _type == Type::NONE;
		}

		/** Type of the vector */
		Type type() const {
			return _type;
		}

		/** Cast to type T. type() must match. */
		template <class T> std::vector<T>& as();

		/** Cast to type T. type() must match. */
		template <class T> const std::vector<T>& as() const;

        typedef double double_t;
        typedef int64_t int_t;

		/** Push back a value, converting it to an appropriate type. Throw if the value is outside the supported range or if ObjectVector is null. */
		template <class I> void push_back(I v) {
			switch (_type) {
			case Type::DOUBLE:
				as<double>().push_back(MathUtils::safe_cast<double>(v));
				break;
			case Type::FLOAT:
				as<float>().push_back(MathUtils::safe_cast<float>(v));
				break;
			case Type::INT8:
				as<int8_t>().push_back(MathUtils::safe_cast<int8_t>(v));
				break;
			case Type::INT16:
				as<int16_t>().push_back(MathUtils::safe_cast<int16_t>(v));
				break;
			case Type::INT32:
				as<int32_t>().push_back(MathUtils::safe_cast<int32_t>(v));
				break;
			case Type::UINT8:
				as<uint8_t>().push_back(MathUtils::safe_cast<uint8_t>(v));
				break;
			case Type::UINT16:
				as<uint16_t>().push_back(MathUtils::safe_cast<uint16_t>(v));
				break;
			case Type::UINT32:
				as<uint32_t>().push_back(MathUtils::safe_cast<uint32_t>(v));
				break;
			case Type::NONE:
				throw std::domain_error("ObjectVector: pushing back to null");
			default:
				throw_unknown_type(_type);
			}
		}


		/** Reserve capacity for values.
		@throw std::domain_error If type == NONE */
		void reserve(size_t capacity);

		/** Resize vector 
		@throw std::domain_error If type == NONE */
		void resize(size_t new_size);

		// get ObjectVector type constant for given value type
		template <class V> static Type get_type();

		friend std::ostream& operator<<(std::ostream& os, ObjectVector::Type typ);

		friend std::ostream& operator<<(std::ostream& os, const ObjectVector& v);
	private:
		/** Copy initialisation */
		template <class T> void init(const std::vector<T>& value, Type type);

		static void throw_unknown_type(ObjectVector::Type type);

		void* _value;
		Type _type;

	};

	inline void swap(ObjectVector& l, ObjectVector& r) {
		l.swap(r);
	}
    
}

/*
 * (C) Averisera Ltd 2014
 */
#ifndef __AVERISERA_OBJECT_VALUE_H
#define __AVERISERA_OBJECT_VALUE_H

#include <cassert>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace averisera {

	/** @brief Class holding a value of variable type

      TODO: refactor it to make the code shorter similar to ObjectVector
	*/
	class ObjectValue {
	public:
		/**
		Type of the object value. NONE should be last.
		*/
		enum class Type : int8_t { 
			DOUBLE, /**< double value */
			INT, /**< int value */
			BOOL, /**< bool value */
			STRING, /**< std::string value */
			OBJECT, /**< class object (held via shared pointer) */
			VECTOR, /**< Vector of ObjectValue objects */
			NONE /**< No value */
		};

		ObjectValue(double x);

		ObjectValue(int i);

		ObjectValue(bool b);

		ObjectValue(const std::string& s);

		ObjectValue(const char* s);

		ObjectValue(const std::vector<ObjectValue>& vector);

		static ObjectValue from_object(std::shared_ptr<void> o);

		// Default to NULL
		ObjectValue();

		ObjectValue(ObjectValue&& other);

		ObjectValue(const ObjectValue& other);

		ObjectValue& operator=(const ObjectValue& other);

		~ObjectValue();

		const std::string& as_string() const {
			assert( _type == Type::STRING );
			return *static_cast<const std::string *>(_value);
		}

		// Return value as bool
		bool as_bool() const {
			assert(_type == Type::BOOL);
			return *static_cast<const bool *>(_value);
		}

		// Return value as int
		int as_int() const {
			assert(_type == Type::INT);
			return *static_cast<const int *>(_value);
		}

		// Return value as double
		double as_double() const { 
			assert(_type == Type::DOUBLE);
			return *static_cast<const double *>(_value);
		}

		const std::vector<ObjectValue>& as_vector() const {
			assert(_type == Type::VECTOR);
			return *static_cast<const std::vector<ObjectValue>*>(_value);
		}

		std::vector<ObjectValue>& as_vector() {
			assert(_type == Type::VECTOR);
			return *static_cast<std::vector<ObjectValue>*>(_value);
		}

		/** Convert std::vector<ObjectValue> to std::vector<double>
		Type must be VECTOR, and all ObjectValue values inside it must have type DOUBLE.
		*/
        std::vector<double> as_double_vector() const;

		/** Return value as shared pointer */
		template <class T> std::shared_ptr<T> as_object() const {
			assert(_type == Type::OBJECT);
			return std::static_pointer_cast<T, void>(*static_cast<std::shared_ptr<void>*>(_value));
		}

		// Return the type of the object value
		Type type() const {
			return _type; 
		}

		friend void swap(ObjectValue& left, ObjectValue& right) {
			using std::swap;
			swap(left._type, right._type);
			swap(left._value, right._value);
		}		
	private:
		// initialise the contents by creating a copy of the value
		template <class T> void init(const T& value, Type type);

		// initialise the contents by moving the value
		template <class T> void init_move(T&& value, Type type);

		void* _value;
		Type _type;
	};

	std::ostream& operator<<(std::ostream& os, const ObjectValue::Type typ);
}

#endif

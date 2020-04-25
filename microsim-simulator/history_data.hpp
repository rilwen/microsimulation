#pragma once
#include "core/dates.hpp"
#include "core/object_vector.hpp"
#include <cassert>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

namespace averisera {
	struct Period;
	namespace microsim {
		/** Data to create History implementations from.
		@see History */
		class HistoryData {
		public:
			typedef std::vector<Date>::iterator date_iterator;
			typedef std::vector<Date>::const_iterator const_date_iterator;
            typedef ObjectVector::Type type_t; /**< Type of stored values */

            /** Convert string name to value type enum.
              @throw std::domain_error If name unsupported. */
            static type_t type_from_string(const std::string& str) {
                return ObjectVector::type_from_string(str);
            }

			/** Create empty data
			@param factory_type HistoryFactory type used to build the History (keep it so that we can recreate the History from HistoryData)
			@param name History name
			*/
            HistoryData(const std::string& factory_type, const std::string& name);

			/** Empty data from DENSE factory */
			HistoryData(const std::string& name, type_t typ)
				: HistoryData(value_typ_to_str(typ), name) {}

			HistoryData()
				: HistoryData("double", "") {}

			HistoryData(HistoryData&& other) noexcept;

			HistoryData& operator=(HistoryData&& other) noexcept;

			HistoryData(const HistoryData&);
			HistoryData& operator=(const HistoryData&);

			void swap(HistoryData& other) noexcept;

			/** Return iterator to dates element with given date, or dates.end() if not found. */
			date_iterator find_by_date(Date date);

			/** Return iterator to dates element with given date, or dates.end() if not found. */
			const_date_iterator find_by_date(Date date) const;

			/**
			Change old_date to new_date if doing so doesn't reorder the values. Fail if old_date is not present.
			@return true if succeeded, false if failed.
			*/
			bool change_date_safely(const Date old_date, const Date new_date);

			void set_factory_type(const std::string& new_factory_type) {
				factory_type_ = new_factory_type;
			}

			/** Clear the data */
			void clear();

			/** Append to history the data read from str. If the first character of the string is 'D', read the rest to TimeSeries<Date, History::double_t> and
			append each element. If the first character is 'I', read the rest to TimeSeries<Date, History::int_t> and append each element.
			If string is empty, do nothing.
			@return number of elements read
			@throw std::runtime_error If cannot parse the string or dates in it are not strictly increasing. */
			size_t append(const std::string& str);

            /** Append value */
            template <class V> HistoryData& append(Date date, V value) {
                dates_.push_back(date);
                values_.push_back(value);
				return *this;
            }
            
            /** Number of (date, value) pairs */
            size_t size() const {
                assert(dates_.size() == values_.size());
                return dates_.size();
            }

			void reserve(size_t capacity) {
				dates_.reserve(capacity);
				values_.reserve(capacity);
			}

			/** Resize to a new smaller size, or leave size intact if new_size > size() */
			void truncate_to(size_t new_size) {
				if (new_size < size()) {
					dates_.resize(new_size);
					values_.resize(new_size);
				}
			}

			const std::vector<Date>& dates() const {
				return dates_;
			}

			const ObjectVector& values() const {
				return values_;
			}

			template <class V> HistoryData& set_value(size_t idx, V value) {
				assert(idx < size());
				values_.as<V>()[idx] = value;
				return *this;
			}

			/** Return HistoryFactory type */
			const std::string& factory_type() const {
				return factory_type_;
			}

			/** Return history name */
			const std::string& name() const {
				return name_;
			}

			/** Shift all dates by delta or throw if not possible due to range constraints
			@throw std::runtime_error */
			void shift_dates(const Period& delta);

			friend std::ostream& operator<<(std::ostream& os, const HistoryData& data);
		private:
			ObjectVector values_;
			std::vector<Date> dates_;
			std::string factory_type_; /** Type of HistoryFactory */
			std::string name_; /** History name */

			template <class V> size_t append_impl(const std::string& str);

			static std::string value_typ_to_str(type_t typ);
		};        

		inline void swap(HistoryData& l, HistoryData& r) {
			l.swap(r);
		}
	}
}

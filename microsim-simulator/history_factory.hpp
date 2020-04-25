/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_HISTORY_FACTORY_H
#define __AVERISERA_MS_HISTORY_FACTORY_H

#include <cstdint>
#include <memory>
#include "history/history_sparse.hpp"
#include "history/history_time_series.hpp"

namespace averisera {
	namespace microsim {
		class History;
		class HistoryData;

		/** @brief Factory methods to make History objects.
		*/
		class HistoryFactory {
        public:
            /** Factory function type 
			std::string parameter is the created history's name */
            typedef std::unique_ptr<History>(*factory_t)(const std::string&);

            /** Build a sparse History.
              
			@param[in] impl Pointer to implementation backing the sparse History. Created object takes over the ownership of impl.
			@throw std::domain_error If impl is null
			@return Pointer to instance
			*/
			static std::unique_ptr<History> make_sparse(std::unique_ptr<History>&& impl);

            /** Return a factory building sparse histories backed by TimeSeries */
            template <class T> static factory_t SPARSE();

            /** Return a factory building dense histories backed by TimeSeries */
            template <class T> static factory_t DENSE();

            /** Append to given history data read from str. If the first character of the string is 'D', read the rest to TimeSeries<Date, History::double_t> and
              append each element. If the first character is 'I', read the rest to TimeSeries<Date, History::int_t> and append each element.
              If string is empty, do nothing.
              @return number of elements read
              @throw std::runtime_error If cannot parse the string */
            static History::index_t append(History& history, const std::string& str);

            /** Create a HistoryFactory function from string representation.
              @throw std::runtime_error If cannot parse the string 
              @return Pair of (factory, value type name)
            */
            static std::pair<HistoryFactory::factory_t, std::string> from_string(const std::string& str);

			/** Make a history factory, use it to create a History and append data to it. 
			@param history_name History name
			*/
			static std::unique_ptr<History> from_strings(const std::string& factory_str, const std::string& data_str,
				const std::string& history_name);

			/** Make a history from HistoryData object. Clear data. */
			static std::unique_ptr<History> from_data(HistoryData&& data);			

			/** Append dates and values from HistoryData object to an existing History. Clear data.
			@return number of elements read */
			static History::index_t append(History& history, HistoryData&& data);
		};

		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<float>();

        template <> HistoryFactory::factory_t HistoryFactory::SPARSE<double>();

		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<int8_t>();

		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<int16_t>();

		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<int32_t>();

		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<uint8_t>();

		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<uint16_t>();

		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<uint32_t>();

		template <> HistoryFactory::factory_t HistoryFactory::DENSE<float>();

        template <> HistoryFactory::factory_t HistoryFactory::DENSE<double>();

		template <> HistoryFactory::factory_t HistoryFactory::DENSE<int8_t>(); 

		template <> HistoryFactory::factory_t HistoryFactory::DENSE<int16_t>();

		template <> HistoryFactory::factory_t HistoryFactory::DENSE<int32_t>();

        template <> HistoryFactory::factory_t HistoryFactory::DENSE<uint8_t>();

		template <> HistoryFactory::factory_t HistoryFactory::DENSE<uint16_t>();

		template <> HistoryFactory::factory_t HistoryFactory::DENSE<uint32_t>();
	}
}

#endif // AVERISERA_MS_HISTORY_FACTORY_H

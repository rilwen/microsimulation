/*
* (C) Averisera Ltd 2015
*/
#include "history.hpp"
#include "history_data.hpp"
#include "history_factory.hpp"
#include "history/history_sparse.hpp"
#include "history/history_time_series.hpp"
#include <stdexcept>
#include "core/math_utils.hpp"
#include "core/object_vector.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

namespace averisera {
	namespace microsim {
        std::unique_ptr<History> HistoryFactory::make_sparse(std::unique_ptr<History>&& impl) {
            return std::unique_ptr<History>(new HistorySparse(std::move(impl)));
        }

        template <class T> static std::unique_ptr<History> make_backed_by_time_series(const std::string& name) {
            return std::unique_ptr<History>(new HistoryTimeSeries<T>(name));
        }

        //template std::unique_ptr<History> make_backed_by_time_series<double>(const std::string& name);

        /*std::unique_ptr<History> make_backed_by_time_series_double(const std::string& name) {
            return make_backed_by_time_series<double>(name);
        }

        std::unique_ptr<History> make_backed_by_time_series_int32(const std::string& name) {
            return make_backed_by_time_series<int32_t>(name);
        }

		std::unique_ptr<History> make_backed_by_time_series_uint32(const std::string& name) {
			return make_backed_by_time_series<uint32_t>(name);
		}

        std::unique_ptr<History> make_backed_by_time_series_int8(const std::string& name) {
            return make_backed_by_time_series<int8_t>(name);
        }

        std::unique_ptr<History> make_backed_by_time_series_uint8(const std::string& name) {
            return make_backed_by_time_series<uint8_t>(name);
        }*/

        template <class T> static std::unique_ptr<History> make_sparse_backed_by_time_series(const std::string& name) {
            return HistoryFactory::make_sparse(std::unique_ptr<History>(new HistoryTimeSeries<T>(name)));
        }

        /*std::unique_ptr<History> make_sparse_backed_by_time_series_double(const std::string& name) {
            return make_sparse_backed_by_time_series<double>(name);
        }

		std::unique_ptr<History> make_sparse_backed_by_time_series_float(const std::string& name) {
			return make_sparse_backed_by_time_series<float>(name);
		}

        std::unique_ptr<History> make_sparse_backed_by_time_series_int32(const std::string& name) {
            return make_sparse_backed_by_time_series<int32_t>(name);
        }

		std::unique_ptr<History> make_sparse_backed_by_time_series_uint32(const std::string& name) {
			return make_sparse_backed_by_time_series<uint32_t>(name);
		}

        std::unique_ptr<History> make_sparse_backed_by_time_series_int8(const std::string& name) {
            return make_sparse_backed_by_time_series<int8_t>(name);
        }

        std::unique_ptr<History> make_sparse_backed_by_time_series_uint8(const std::string& name) {
            return make_sparse_backed_by_time_series<uint8_t>(name);
        }*/

		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<float>() {
			return make_sparse_backed_by_time_series<float>;
		}

		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<double>() {
			return make_sparse_backed_by_time_series<double>;
		}

		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<int8_t>() {
			return make_sparse_backed_by_time_series<int8_t>;
		}
		
		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<int16_t>() {
			return make_sparse_backed_by_time_series<int16_t>;
		}

		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<int32_t>() {
            return make_sparse_backed_by_time_series<int32_t>;
        }
        
		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<uint8_t>() {
            return make_sparse_backed_by_time_series<uint8_t>;
        }

		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<uint16_t>() {
			return make_sparse_backed_by_time_series<uint16_t>;
		}

		template <> HistoryFactory::factory_t HistoryFactory::SPARSE<uint32_t>() {
			return make_sparse_backed_by_time_series<uint32_t>;
		}

        template <> HistoryFactory::factory_t HistoryFactory::DENSE<float>() {
			return make_backed_by_time_series<float>;
		}

		template <> HistoryFactory::factory_t HistoryFactory::DENSE<double>() {
			return make_backed_by_time_series<double>;
		}

		template <> HistoryFactory::factory_t HistoryFactory::DENSE<int8_t>() {
			return make_backed_by_time_series<int8_t>;
		}

		template <> HistoryFactory::factory_t HistoryFactory::DENSE<int16_t>() {
			return make_backed_by_time_series<int16_t>;
		}

		template <> HistoryFactory::factory_t HistoryFactory::DENSE<int32_t>() {
            return make_backed_by_time_series<int32_t>;
        }

        template <> HistoryFactory::factory_t HistoryFactory::DENSE<uint8_t>() {
            return make_backed_by_time_series<uint8_t>;
        }

		template <> HistoryFactory::factory_t HistoryFactory::DENSE<uint16_t>() {
			return make_backed_by_time_series<uint16_t>;
		}

		template <> HistoryFactory::factory_t HistoryFactory::DENSE<uint32_t>() {
			return make_backed_by_time_series<uint32_t>;
		}

        static std::pair<HistoryFactory::factory_t, std::string> from_elements(std::vector<std::string>::const_iterator begin, std::vector<std::string>::const_iterator end) {
            if (begin == end) {
                return std::make_pair(nullptr, "");
            }
            HistoryFactory::factory_t factory;
            if (*begin == "sparse") {
                ++begin;
                if (begin == end) {
                    return std::make_pair(nullptr, "");
                }
				if (*begin == print_type_name<double>()) {
					factory = HistoryFactory::SPARSE<double>();
				} else if (*begin == print_type_name<float>()) {
					factory = HistoryFactory::SPARSE<float>();
				} else if (*begin == print_type_name<int32_t>()) {
                    factory = HistoryFactory::SPARSE<int32_t>();
				} else if (*begin == print_type_name<int16_t>()) {
					factory = HistoryFactory::SPARSE<int16_t>();
				} else if (*begin == print_type_name<int8_t>()) {
                    factory = HistoryFactory::SPARSE<int8_t>();
                } else if (*begin == print_type_name<uint8_t>()) {
                    factory = HistoryFactory::SPARSE<uint8_t>();
				} else if (*begin == print_type_name<uint16_t>()) {
					factory = HistoryFactory::SPARSE<uint16_t>();
				} else if (*begin == print_type_name<uint32_t>()) {
					factory = HistoryFactory::SPARSE<uint32_t>();
				} else {
                    throw std::runtime_error(boost::str(boost::format("HistoryFactory: unknown type \"%s\"") % (*begin)));
                }
            } else {
				if (*begin == print_type_name<double>()) {
					factory = HistoryFactory::DENSE<double>();
				} else if (*begin == print_type_name<float>()) {
					factory = HistoryFactory::DENSE<float>();
				} else if (*begin == print_type_name<int32_t>()) {
                    factory = HistoryFactory::DENSE<int32_t>();
				} else if (*begin == print_type_name<int16_t>()) {
					factory = HistoryFactory::DENSE<int16_t>();
				} else if (*begin == print_type_name<int8_t>()) {
                    factory = HistoryFactory::DENSE<int8_t>();
                } else if (*begin == print_type_name<uint8_t>()) {
                    factory = HistoryFactory::DENSE<uint8_t>();
				} else if (*begin == print_type_name<uint16_t>()) {
					factory = HistoryFactory::DENSE<uint16_t>();
				} else if (*begin == print_type_name<uint32_t>()) {
					factory = HistoryFactory::DENSE<uint32_t>();
				} else {
                    throw std::runtime_error(boost::str(boost::format("HistoryFactory: unknown type \"%s\"") % (*begin)));
                }
            }
            return std::make_pair(factory, *begin);
        }

        std::pair<HistoryFactory::factory_t, std::string> HistoryFactory::from_string(const std::string& str) {
            std::string copy(str);
            std::vector<std::string> elements;
            boost::split(elements, copy, boost::is_any_of(" "));
            try {
                std::pair<HistoryFactory::factory_t, std::string> result = from_elements(elements.begin(), elements.end());
                if (!result.first) {
                    throw std::runtime_error(boost::str(boost::format("HistoryFactory: unable to parse \"%s\"") % str));
                }
                return result;
            } catch (std::exception& e) {
                throw std::runtime_error(boost::str(boost::format("HistoryFactory: unable to parse \"%s\": %s") % str % e.what()));
            }
        }

        template <class V> static History::index_t append_impl(History& history, const std::string& str) {
            const TimeSeries<Date, V> ts(TimeSeries<Date, V>::from_string(str));
            for (const auto& date_value: ts) {
                history.append(date_value.first, date_value.second);
            }
            return MathUtils::safe_cast<History::index_t>(ts.size());
        }

        History::index_t HistoryFactory::append(History& history, const std::string& str) {
            if (!str.empty()) {
                switch (str[0]) {
                case 'D':
                    return append_impl<History::double_t>(history, str.substr(1));
                case 'I':
                    return append_impl<History::int_t>(history, str.substr(1));
                default:
                    throw std::runtime_error("HistoryFactory: cannot parse string with history data");
                }
            } else {
                return 0;
            }
        }

		std::unique_ptr<History> HistoryFactory::from_strings(const std::string& factory_str, const std::string& data_str, const std::string& history_name) {
			HistoryFactory::factory_t factory = from_string(factory_str).first;
			std::unique_ptr<History> history = factory(history_name);
			append(*history, data_str);
			return history;
		}

		std::unique_ptr<History> HistoryFactory::from_data(HistoryData&& data) {
			HistoryFactory::factory_t factory = from_string(data.factory_type()).first;
			std::unique_ptr<History> history = factory(data.name());
			append(*history, std::move(data));
			return history;
		}

		template <class T, class V> static size_t append_impl(History& history, const std::vector<Date>& dates, const std::vector<V>& values) {
			const size_t size = dates.size();
			assert(size == values.size());
			auto date_it = dates.begin();
			auto value_it = values.begin();
			while (date_it != dates.end()) {
				assert(value_it != values.end());
				history.append(*date_it, MathUtils::safe_cast<T>(*value_it));
				++date_it;
				++value_it;
			}
			return size;
		}

		// TODO: test it thoroughly
		History::index_t HistoryFactory::append(History& history, HistoryData&& data) {			
			size_t cnt = 0;
			switch (data.values().type()) {
			case ObjectVector::Type::DOUBLE:
				cnt = append_impl<History::double_t>(history, data.dates(), data.values().as<double>());
				break;
			case ObjectVector::Type::FLOAT:
				cnt = append_impl<History::double_t>(history, data.dates(), data.values().as<float>());
				break;
			case ObjectVector::Type::INT8:
				cnt = append_impl<History::int_t>(history, data.dates(), data.values().as<int8_t>());
				break;
			case ObjectVector::Type::INT16:
				cnt = append_impl<History::int_t>(history, data.dates(), data.values().as<int16_t>());
				break;
			case ObjectVector::Type::INT32:
				cnt = append_impl<History::int_t>(history, data.dates(), data.values().as<int32_t>());
				break;
			case ObjectVector::Type::UINT8:
				cnt = append_impl<History::int_t>(history, data.dates(), data.values().as<uint8_t>());
				break;
			case ObjectVector::Type::UINT16:
				cnt = append_impl<History::int_t>(history, data.dates(), data.values().as<uint16_t>());
				break;
			case ObjectVector::Type::UINT32:
				cnt = append_impl<History::int_t>(history, data.dates(), data.values().as<uint32_t>());
				break;
			case ObjectVector::Type::NONE:
				cnt = 0;
				break;
			default:
				throw std::logic_error(boost::str(boost::format("HistoryFactory: unknown ObjectVector type: %d") % static_cast<int>(data.values().type())));
			}
			data.clear();
			return cnt;
		}

    }
}

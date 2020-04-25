/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_TIME_SERIES_H
#define __AVERISERA_TIME_SERIES_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <vector>
#include <utility>
#include "dates.hpp"
#include "math_utils.hpp"
#include "segment_search.hpp"
#include "utils.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

namespace averisera {
    /** @brief Series of values indexed by time
      
     Series of values indexed by time, containing functions allowing fast retrieval of data.
     Assumes that times are strictly increasing.

     Data are stored as a vector of time,value pairs to improve locality and allow easy iteration.

	 Values are mutable in a non-const TimeSeries, but dates are not.
     
     @tparam T Time time. Copied by value.
     @tparam V Value type. Copied by value. Defaults to double.
     */
    template <class T, class V = double> class TimeSeries {
    public:
        typedef std::pair<T, V> tvpair_t; /**< Time-value pair type 
										  T is often Date, which is 32-bit, so should go first for smaller object size
										  */
		typedef size_t index_t;
        
        /** Construct an empty time series */
        TimeSeries() 
            : _size(0) {
        }
        
		/** Construct a time series with single element */
        TimeSeries(T time, const V& value)
            : _times_values(1, std::make_pair(time, value)), _size(1) {
        }

		/** Construct a time series with single element by moving the value */
		TimeSeries(T time, V&& value)
			: _times_values(1, std::make_pair(time, std::move(value))), _size(1) {
		}
        
        /** Construct a time series from precalculated data. 
         * Both vectors must have equal size. The times must be strictly increasing.
         * 
         * @param[in] times Time vector
         * @param[in] values Value vector
         * @throw std::domain_error If parameters do not satify the preconditions.
         */
        TimeSeries(const std::vector<T>& times, const std::vector<V>& values);

        /** Construct a time series from precalculated data. 
         * The times must be strictly increasing.
         * 
         * @param[in] times_values (Time, value) pair vector
         * @throw std::domain_error If parameters do not satify the preconditions.
         */
        explicit TimeSeries(const std::vector<tvpair_t>& times_values);

        /** Move-Construct a time series from precalculated data. 
         * The times must be strictly increasing.
         * 
         * @param[in] times_values (Time, value) pair vector
         * @throw std::domain_error If parameters do not satify the preconditions.
         */
        TimeSeries(std::vector<tvpair_t>&& times_values);
        
        /** @brief Copy constructor */
        TimeSeries(const TimeSeries<T, V>& other) = default;
        
        /** Modified copy constructor which copies just a subset of the other time series with indices in [begin, end) range.
         *        @throw std::domain_error If begin >= other.size(), end < begin or end > other.size()
         */
        TimeSeries(const TimeSeries<T, V>& other, const index_t begin, index_t end)
        : _size(end - begin)
        {
            if (begin >= other.size() || end < begin || end > other.size()) {
                throw std::domain_error("TimeSeries: bad index range");
            }
            _times_values.resize(_size);
            std::copy(other._times_values.begin() + begin, other._times_values.begin() + end, _times_values.begin());
        }
        
        /** @brief Move constructor */
        TimeSeries(TimeSeries<T, V>&& other) noexcept
        : _times_values(std::move(other._times_values)), _size(other._size) {
            // Bravely assume that other._times and other._values will be empty after the move.
            other._size = 0;
        }
        
        TimeSeries<T, V>& operator=(const TimeSeries<T, V>& other) {
            TimeSeries<T, V> clone(other);
            clone.swap(*this);
            return *this;
        }

        /** Default move assignment operator */
        TimeSeries<T, V>& operator=(TimeSeries<T, V>&& other) {
            _times_values = std::move(other._times_values);
            _size = other._size;
            other._size = 0;
            return *this;
        }
        
        void swap(TimeSeries<T, V>& other) {
            _times_values.swap(other._times_values);
            std::swap(_size, other._size);
        }
        
        /** Length of the time series. */
        index_t size() const {
            return _size;
        }
        
        /** Is it empty */
        bool empty() const {
            return _size == 0;
        }

        /** Reserve memory for adding N elements */
        void reserve(index_t N) {
            _times_values.reserve(N);
        }

        // /** Const ref to (time, value) pair vector */
        // const std::vector<tvpair_t>& times_values() const {
        //     return _times_values;
        // }
        
        /** Append a time and value at the end of the time series.
         * New time must be past the last time of the series and cannot be a "not a time".
         * 
         * @param[in] time New time
         * @param[in] value New value
         * @throw std::domain_error If parameters do not satify the preconditions.
         */
        void push_back(T time, V value) {
            push_back(std::make_pair(time, value));
        }

        /** Append a time and value at the end of the time series.
         * New time must be past the last time of the series and cannot be a "not a time".
         * 
         * @param[in] time_value New time,value pair
         * @throw std::domain_error If parameters do not satify the preconditions.
         */
        void push_back(const std::pair<T, V>& time_value);     
        
        /** First value if not empty 
         * @throw std::logic_error If time series is empty.
         */
        V first_value() const;
        
        /** First time if not empty 
         * @throw std::logic_error If time series is empty.
         */
        T first_time() const;
        
        /** First (time, value) pair if not empty
         * @throw std::logic_error if time series is empty
         */
        const std::pair<T, V>& first() const;
        
        /** Last value if not empty 
         * @throw std::logic_error If time series is empty.
         */
        V last_value() const;
        
        /** Last time if not empty 
         * @throw std::logic_error If time series is empty.
         */
        T last_time() const;
        
        /** Last (time, value) pair if not empty
         * @throw std::logic_error if time series is empty
         */
        const std::pair<T, V>& last() const;
        
        /** Value for given time.
         * 
         * @param[in] time Time value
         * @return Pointer to value if found, or nullptr if not.
         */
        const V* value(T time) const;

		/** Value for given time.
		*
		* @param[in] time Time value
		* @return Pointer to value if found, or nullptr if not.
		*/
		V* value(T time);
        
        /** Last value on or before given time.
         * 
         * @param[in] time Time value
         * @return Pointer to value if found, or nullptr if not.
         */
        const V* last_value(T time) const;
        
        /** Last time on or before given time.
         * 
         * @param[in] time Time value
         * @return Pointer to time if found, or nullptr if not.
         */
        const T* last_time(T time) const;

        /** Last (time, value) pair on or before given time.
         * 
         * @param[in] time Time value
         * @return Pointer to time, value if found, or nullptr if not.
         */
        const tvpair_t* last_time_value(T time) const;
        
        /** Last index on or before given time
         * 
         * @param[in] time Time value
         * @throw std::out_of_range If time given is before the first time in the series.
         */
        index_t last_index(T time) const;

        /** First index on or after given time
         * 
         * @param[in] time Time value
         * @throw std::out_of_range If time given is after the last time in the series.
         */
        index_t first_index(T time) const;

        /** Const indexing operator. For speed reasons does not do bounds checks. */
        const tvpair_t& operator[](index_t idx) const {
            assert(idx < _size);
            return _times_values[idx];
        }

        /** Get reference to value at index. For speed reasons does not do bounds checks. */
        V& value_at_index(index_t idx) {
            assert(idx < _size);
            return _times_values[idx].second;
        }
       
        typedef typename std::vector<tvpair_t>::const_iterator const_iterator;

        const_iterator begin() const {
            return _times_values.begin();
        }

        const_iterator end() const {
            return _times_values.end();
        }

        /** Prints TimeSeries to a text representation from which it can be reconstructed "most of the time". */
        friend std::ostream& operator<<(std::ostream& os, const TimeSeries& ts) {
            os << '[';
            for (index_t i = 0; i < ts._size; ++i) {
                if (i > 0) {
                    os << '|';
                }
                os << ts._times_values[i].first << "," << static_cast<typename Utils::printable_type<V>::type>(ts._times_values[i].second);
            }
            os << ']';
            return os;
        }

        /** Read TimeSeries from string assuming it's in the same format as the one generated by << operator. */
        static TimeSeries<T, V> from_string(const std::string& str);

        bool operator==(const TimeSeries<T, V>& other) const {
            if (this == &other) {
                return true;
            }
            return _times_values == other._times_values;
        }		

		/** Find value for any time, extrapolating forward between and after last value, and backward before first value. 
		@param time Any time 
		@param value[0] if time <= time[0], value[i] for i <= time < i + 1 and last value for larger time.
		*/
		V padded_value(T time) const {
			const size_t idx = SegmentSearch::find_index_for_padding_forward_and_backward(_times_values, _size, time, time_getter);
			return _times_values[idx].second;
		}

		template <class V2> TimeSeries<T, V>& operator*=(const V2& x);

		class ValueIterator: public std::iterator<std::random_access_iterator_tag, V> {
			// TODO: finish implementing the methods and operators
		public:
			friend TimeSeries<T, V>;
			ValueIterator(const ValueIterator&) = default;
			ValueIterator& operator=(const ValueIterator&) = default;
			friend ValueIterator& operator++(ValueIterator& iter) {
				++iter.pair_iter_;
				return iter;
			}
            
			typename ValueIterator::value_type& operator*() const {
				return pair_iter_->second;
			}
            
            typename ValueIterator::value_type* operator->() const {
				return &(pair_iter_->second);
			}
            
			bool operator==(const ValueIterator& other) const {
				return pair_iter_ == other.pair_iter_;
			}
            
			bool operator!=(const ValueIterator& other) const {
				return pair_iter_ != other.pair_iter_;
			}
            
			bool operator<(const ValueIterator& other) const {
				return pair_iter_ < other.pair_iter_;
			}
            
            typename ValueIterator::difference_type operator-(const ValueIterator& other) const {
				return pair_iter_ - other.pair_iter_;
			}
		private:
			ValueIterator(typename std::vector<tvpair_t>::iterator iter)
				: pair_iter_(iter) {}
			typename std::vector<tvpair_t>::iterator pair_iter_;
		};

		ValueIterator values_begin() {
			return ValueIterator(_times_values.begin());
		}

		ValueIterator values_end() {
			return ValueIterator(_times_values.end());
		}
    private:
        static void check_inputs(const std::vector<tvpair_t>& times_values);

        static bool compare_lower(const tvpair_t& p, T t) {
            return p.first < t;
        }

        static bool compare_upper(T t, const tvpair_t& p) {
            return t < p.first;
        }
        //static void check_inputs(const std::vector<T>& times, const std::vector<V>& values);

		static T time_getter(const std::vector<tvpair_t>& vec, size_t idx) {
			return vec[idx].first;
		}
        
        std::vector<tvpair_t> _times_values;
        index_t _size;
    };
    
    template <class T> bool is_not_a_time(T /*time*/) {
        return false; // by default all time values are valid
    }
    
    template <> bool is_not_a_time<double>(double time);
    
    template <> bool is_not_a_time<Date>(Date time);

    template <class T, class V> TimeSeries<T,V>::TimeSeries(const std::vector<T>& times, const std::vector<V>& values) {
        if (values.size() != times.size()) {
            throw std::domain_error("TimeSeries: times and values vectors have different sizes");
        }
        std::vector<tvpair_t> times_values;
        times_values.reserve(times.size());
        auto t_it = times.begin();
        auto v_it = values.begin();
        while (t_it != times.end()) {
            assert(v_it != values.end());
            times_values.push_back(std::make_pair(*t_it, *v_it));
            ++t_it;
            ++v_it;
        }
        assert(t_it == times.end());
        assert(v_it == values.end());
        check_inputs(times_values);
        _size = static_cast<index_t>(times.size());
        _times_values = std::move(times_values);
    }

    template <class T, class V> TimeSeries<T,V>::TimeSeries(const std::vector<tvpair_t>& times_values) {
        check_inputs(times_values);
        _size = static_cast<index_t>(times_values.size());
        _times_values = times_values;
    }

    template <class T, class V> TimeSeries<T,V>::TimeSeries(std::vector<tvpair_t>&& times_values) {
        check_inputs(times_values);
        _size = static_cast<index_t>(times_values.size());
        _times_values = std::move(times_values);
    }

    template <class T, class V> void TimeSeries<T,V>::push_back(const std::pair<T, V>& time_value) {
        if (is_not_a_time(time_value.first)) {
            throw std::domain_error("TimeSeries: appending not-a-time");
        }
        if (!_times_values.empty() && time_value.first <= _times_values.back().first) {
            throw std::domain_error("TimeSeries: appended time not past the last time");
        }        
        _times_values.push_back(time_value);
        ++_size;
    }
    
    template <class T, class V> V TimeSeries<T,V>::last_value() const {
        if (empty()) {
            throw std::logic_error("TimeSeries is empty");
        } else {
            return _times_values.back().second;
        }
    }
    
    template <class T, class V> T TimeSeries<T,V>::last_time() const {
        if (empty()) {
            throw std::logic_error("TimeSeries is empty");
        } else {
            return _times_values.back().first;
        }
    }
    
    template <class T, class V> const std::pair<T,V>& TimeSeries<T,V>::last() const {
        if (empty()) {
            throw std::logic_error("TimeSeries is empty");
        } else {
            return _times_values.back();
        }
    }
    
    template <class T, class V> V TimeSeries<T,V>::first_value() const {
        if (empty()) {
            throw std::logic_error("TimeSeries is empty");
        } else {
            return _times_values.front().second;
        }
    }
    
    template <class T, class V> T TimeSeries<T,V>::first_time() const {
        if (empty()) {
            throw std::logic_error("TimeSeries is empty");
        } else {
            return _times_values.front().first;
        }
    }
    
    template <class T, class V> const std::pair<T,V>& TimeSeries<T,V>::first() const {
        if (empty()) {
            throw std::logic_error("TimeSeries is empty");
        } else {
            return _times_values.front();
        }
    }
    
    template <class T, class V> const V* TimeSeries<T,V>::value(T time) const {
        const auto it = std::lower_bound(_times_values.begin(), _times_values.end(), time, compare_lower);
        if (it != _times_values.end() && it->first == time) {
            return &(it->second);
        } else {
            return nullptr;
        }
    }

	template <class T, class V> V* TimeSeries<T, V>::value(T time) {
		const auto it = std::lower_bound(_times_values.begin(), _times_values.end(), time, compare_lower);
		if (it != _times_values.end() && it->first == time) {
			return &(it->second);
		} else {
			return nullptr;
		}
	}
    
    template <class T, class V> const V* TimeSeries<T,V>::last_value(T time) const {
        auto it = std::upper_bound(_times_values.begin(), _times_values.end(), time, compare_upper);
        if (it != _times_values.begin()) {
            --it;
            return &(it->second);
        } else {
            return nullptr;
        }
    }
    
    template <class T, class V> const T* TimeSeries<T,V>::last_time(T time) const {
        auto it = std::upper_bound(_times_values.begin(), _times_values.end(), time, compare_upper);
        if (it != _times_values.begin()) {
            --it;
            return &(it->first);
        } else {
            return nullptr;
        }
    }

	template <class T, class V> const typename TimeSeries<T, V>::tvpair_t* TimeSeries<T, V>::last_time_value(T time) const {
		auto it = std::upper_bound(_times_values.begin(), _times_values.end(), time, compare_upper);
		if (it != _times_values.begin()) {
			--it;
			return &(*it);
		}
		else {
			return nullptr;
		}
	}
    
    template <class T, class V> typename TimeSeries<T,V>::index_t TimeSeries<T, V>::last_index(T time) const {
        const auto it = std::upper_bound(_times_values.begin(), _times_values.end(), time, compare_upper);
        if (it != _times_values.begin()) {            
            return static_cast<index_t>(it - _times_values.begin() - 1);
        } else {
            throw std::out_of_range("TimeSeries: time given before first time");
        }
    }

    template <class T, class V> typename TimeSeries<T, V>::index_t TimeSeries<T, V>::first_index(T time) const {
        const auto it = std::lower_bound(_times_values.begin(), _times_values.end(), time, compare_lower);
        if (it != _times_values.end()) {            
            return static_cast<index_t>(it - _times_values.begin());
        } else {
            throw std::out_of_range("TimeSeries: time given after last time");
        }
    }

    // template <class T, class V> void TimeSeries<T, V>::check_inputs(const std::vector<T>& times, const std::vector<V>& values) {
    //     if (values.size() != times.size()) {
    //         throw std::domain_error("TimeSeries: times and values vectors have different sizes");
    //     }
    //     if (!times.empty()) {
    //         T prev = times.front();
    //         if (is_not_a_time(prev)) {
    //             throw std::domain_error("TimeSeries: first time is not-a-time");
    //         }
    //         auto next_it = times.begin() + 1;
    //         while (next_it < times.end()) {
    //             T next = *next_it;
    //             if (is_not_a_time(next)) {
    //                 throw std::domain_error("TimeSeries: time is not-a-time");
    //             }
    //             if (prev >= next) {
    //                 throw std::domain_error("TimeSeries: times are not strictly increasing");
    //             } else {
    //                 prev = next;
    //             }
    //             ++next_it;
    //         }
    //     }        
    // }

    template <class T, class V> void TimeSeries<T, V>::check_inputs(const std::vector<tvpair_t>& times_values) {
        if (!times_values.empty()) {
            tvpair_t prev = times_values.front();
            if (is_not_a_time(prev.first)) {
                throw std::domain_error("TimeSeries: first time is not-a-time");
            }
            auto next_it = times_values.begin() + 1;
            while (next_it < times_values.end()) {
                tvpair_t next = *next_it;
                if (is_not_a_time(next.first)) {
                    throw std::domain_error("TimeSeries: time is not-a-time");
                }
                if (prev.first >= next.first) {
                    throw std::domain_error("TimeSeries: times are not strictly increasing");
                } else {
                    prev = next;
                }
                ++next_it;
            }
        }
    }

    template <class T, class V> TimeSeries<T, V> TimeSeries<T, V>::from_string(const std::string& str) {
        if (str.size() < 2) {
            throw std::runtime_error(boost::str(boost::format("TimeSeries: string too short to parse: %s") % str));
        }
        if (str.front() != '[' || str.back() != ']') {
            throw std::runtime_error("TimeSeries: string not in square brackets");
        }
        if (str.size() == 2) {
            return TimeSeries<T, V>();
        }
        std::vector<std::string> pairs;
        std::string no_brackets(str.substr(1, str.size() - 2));
        boost::split(pairs, no_brackets, boost::is_any_of("|"));
        if (!pairs.empty()) {
            std::vector<tvpair_t> parsed_pairs;
            parsed_pairs.reserve(pairs.size());
            for (const std::string& pair: pairs) {
                const size_t comma_idx = pair.find(',');
                if (comma_idx != std::string::npos) {
                    if (comma_idx == 0) {
                        throw std::runtime_error(boost::str(boost::format("TimeSeries: no time string in pair %s") % pair));
                    } else if (comma_idx + 1 == pair.size()) {
                        throw std::runtime_error(boost::str(boost::format("TimeSeries: no value string in pair %s") % pair));
                    }
                    T t;
                    V v;
                    //std::cout << pair.substr(0, comma_idx) << " : " << pair.substr(comma_idx + 1, pair.size() - (comma_idx + 1)) << std::endl;                    
                    try {
                        t = Utils::from_string<T>(pair.substr(0, comma_idx).c_str());
                    } catch (std::exception& e) {
                        throw std::runtime_error(boost::str(boost::format("TimeSeries: cannot convert string %s to time: %s") % pair.substr(0, comma_idx) % e.what()));
                    }
                    try {
                        v = Utils::from_string<V>(pair.substr(comma_idx + 1, pair.size() - (comma_idx + 1)).c_str());
                    } catch (std::exception& e) {
                        throw std::runtime_error(boost::str(boost::format("TimeSeries: cannot convert string %s to value: %s") % pair.substr(comma_idx + 1, pair.size() - (comma_idx + 1)) % e.what()));
                    }                    
                    //std::cout << "(" << t << "," << v << ")" << std::endl;
                    parsed_pairs.push_back(std::make_pair(t, v));
                } else {
                    throw std::runtime_error(boost::str(boost::format("TimeSeries: element is not a t,v pair: %s") % pair));
                }
            }
            try {
                return TimeSeries<T, V>(std::move(parsed_pairs));
            } catch (std::exception& e) {
                throw std::runtime_error(boost::str(boost::format("TimeSeries: cannot convert string %s to TimeSeries: %s") % str % e.what()));
            }
        } else {
            return TimeSeries<T, V>();
        }		
    }

	template <class T, class V> TimeSeries<T, V> operator*(const TimeSeries<T, V>& ts, const V& x) {
		std::vector<typename TimeSeries<T, V>::tvpair_t> values(ts.begin(), ts.end());
		for (auto& pair : values) {
			pair.second *= x;
		}
		return TimeSeries<T, V>(std::move(values));
	}

	template <class T, class V> TimeSeries<T, V> operator*(const V& x, const TimeSeries<T, V>& ts) {
		return ts * x;
	}

	template <class T, class V> template <class V2>  TimeSeries<T, V>& TimeSeries<T, V>::operator*=(const V2& x) {
		for (auto& pair : _times_values) {
			pair.second *= x;
		}
		return *this;
	}

	template <class T, class V> void swap(TimeSeries<T, V>& l, TimeSeries<T, V>& r) {
		l.swap(r);
	}	
}

#endif // __AVERISERA_TIME_SERIES_H

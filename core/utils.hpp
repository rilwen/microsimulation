/*
(C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_UTILS_H
#define __AVERISERA_UTILS_H

#include <Eigen/Core>
#include "dates.hpp"
#include "preconditions.hpp"
#include "stream_data_output.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace averisera {
	class Daycount;
	struct ObservedDiscreteData;
	struct Period;
	enum class PeriodType;

	namespace Utils {		

		// Load marginal probabilities, numbers of surveys and years the surveys were taken in (if present - if not, sets them to 0, 1, 2, ...) from file fname and store them in columns of matrix p and vector nbr_surveys
		// Row layout is p1, p2, ..., pN, [numbers_surveys], [year_number]
		// Return pair (dim, T)
		std::pair<size_t, size_t> load_probabilities(const char* fname, Eigen::MatrixXd& p, bool check_sum, Eigen::VectorXd& nbr_surveys, std::vector<double>& years);

		// Find such i that upper_bounds[i - 1] < val <= upper_bounds[i]
		// upper_bounds: Sorted vector
		template <class V> size_t range_idx(const std::vector<V>& upper_bounds, const V& val) {
			return std::lower_bound(upper_bounds.begin(), upper_bounds.end(), val) - upper_bounds.begin();
		}		
                
        /** Execute op and return value. Used to pass arguments to super class constructor after performing checks on them */
        template <class T, class F> T& pass_through(T& value, F op) {
            op();
            return value;
        }
		        
        /** Execute op and return value. Used to pass arguments to super class constructor after performing checks on them */
        template <class T, class F> const T& pass_through(const T& value, F op) {
            op();
            return value;
        }

		/** Structure defining "printable" type corresponding to value type. For most arithmetic types printable_type<T>::type == T,
		but for 8-bit types it's int /*/
		template <class T> struct printable_type {
			typedef T type;
		};

		template <> struct printable_type<unsigned char> {
			typedef int type;
		};

		template <> struct printable_type<signed char> {
			typedef int type;
		};

        /** Convert value to string by. Unsigned and signed char are treated as numbers, e.g. char(0) is converted to "0" */
        template <class T> std::string to_string(const T& val) {
            return boost::lexical_cast<std::string>(static_cast<typename printable_type<T>::type>(val));
        }

        /*template <> inline std::string to_string(const signed char& val) {
			return to_string<int>(static_cast<int>(val));
        }

		template <> inline std::string to_string(const unsigned char& val) {
			return to_string<int>(static_cast<int>(val));
		}*/

        template <class T> T from_string(const char* str) {
			try {
				return boost::lexical_cast<T>(str);
			} catch (std::exception& e) {
				throw std::runtime_error(boost::str(boost::format("Utils::from_string: cannot parse %s: %s") % str % e.what()));
			}
        }

		template <> Date from_string<Date>(const char* str);

		template <> PeriodType from_string<PeriodType>(const char* str);

		template <> Period from_string<Period>(const char* str);

		template <> std::shared_ptr<const Daycount> from_string<std::shared_ptr<const Daycount>>(const char* str);

        template <> signed char from_string<signed char>(const char* str);

		template <class T> T from_string(const std::string& str) {
			return from_string<T>(str.c_str());
		}

		template <> Date from_string<Date>(const std::string& str);
        
        template <class M> void save_matrix(std::ofstream& outf, const M& m) {
            const size_t nr = m.rows();
            const size_t nc = m.cols();	
            StreamDataOutput so(&outf, '\t', 14);
            std::vector<double> row(nc);
            for (size_t r = 0; r < nr; ++r) {		
                for (size_t c = 0; c < nc; ++c) {
                    row[c] = m(r, c);
                }
                so.output_data_row(row);
            }
        }

        template <class M> void save_matrix(const char* fname, const M& m) {
            std::ofstream outf(fname);
            save_matrix(outf, m);
        }

        template <class M> void save_matrix(const std::string& fname, const M& m) {
            save_matrix(fname.c_str(), m);
        }
        
        void load_matrix(const std::string& fname, Eigen::MatrixXd& m);
        
        /** Helper function to make std::function from overloaded functions (e.g. exp).*/
		template<typename F> std::function<F> make_function(F* f) {
			return std::function<F>(f);
		}

        /** Convert an element of a map found by key.
		@param convert_empty Whether to try to convert an empty string.
		*/
		template <class V> V from_string_map(const std::unordered_map<std::string, std::string>& map, const std::string& key, const V& default_value, bool convert_empty) {
			const auto iter = map.find(key);
			if (iter != map.end()) {
				if (convert_empty || (!iter->second.empty())) {
					return from_string<V>(iter->second);
				}
			}
			return default_value;
		}
	}
}
    

#endif


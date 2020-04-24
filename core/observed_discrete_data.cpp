/*
  (C) Averisera Ltd 2014
*/
#include "csv_file_reader.hpp"
#include "observed_discrete_data.hpp"
#include "preconditions.hpp"
#include "math_utils.hpp"
#include "utils.hpp"
#include <boost/format.hpp>
#include <cassert>
#include <map>
#include <random>
#include <sstream>

namespace averisera {
	static void ensure_times_size(std::vector<double>& times, const size_t T) {
		if (times.size() == 0) {
			times.resize(T);
			for (size_t t = 0; t < T; ++t) {
				times[t] = static_cast<double>(t);
			}
		}
	}

	ObservedDiscreteData::ObservedDiscreteData(const lcidx_t N, const index_t T)
		: probs(N, T), nbr_surveys(T) {
		nbr_surveys.setOnes();
		ensure_times_size(times, T);
	}

	ObservedDiscreteData::ObservedDiscreteData(const std::vector<std::vector<double>>& times, const std::vector<std::vector<lcidx_t>>& trajs)
		: ltrajs(trajs), ltimes(times) {
		validate();
	}

	ObservedDiscreteData::ObservedDiscreteData(const ObservedDiscreteData& other)
		: probs(other.probs)
		, nbr_surveys(other.nbr_surveys)
		, times(other.times)
		, ltrajs(other.ltrajs)
		, ltimes(other.ltimes)
	{
	}

	ObservedDiscreteData::ObservedDiscreteData(ObservedDiscreteData&& other)
		: probs(std::move(other.probs))
		, nbr_surveys(std::move(other.nbr_surveys))
		, times(std::move(other.times))
		, ltrajs(std::move(other.ltrajs))
		, ltimes(std::move(other.ltimes)) {
		other.probs.resize(0, 0);
		other.nbr_surveys.resize(0);
	}

	ObservedDiscreteData& ObservedDiscreteData::operator=(const ObservedDiscreteData& other) {
		ObservedDiscreteData clone(other);
		swap(clone);
		return *this;
	}

	ObservedDiscreteData& ObservedDiscreteData::operator=(ObservedDiscreteData&& other) {
		ObservedDiscreteData clone(std::move(other));
		swap(clone);
		return *this;
	}

	void ObservedDiscreteData::swap(ObservedDiscreteData& other) {
		probs.swap(other.probs);
		nbr_surveys.swap(other.nbr_surveys);
		times.swap(other.times);
		ltrajs.swap(other.ltrajs);
		ltimes.swap(other.ltimes);
	}

	static ObservedDiscreteData pad_impl(const ObservedDiscreteData& data, std::vector<size_t>& input_to_padded, const double time0, const size_t padded_T) {
		const size_t dim = ObservedDiscreteData::dim(data);
		const std::vector<double>& times = data.times;
		const size_t T = data.nbr_surveys.size();
		if (padded_T > T) {
			ObservedDiscreteData padded_data(data);
			padded_data.probs.resize(dim, padded_T);
			padded_data.nbr_surveys.resize(padded_T);
			padded_data.nbr_surveys.fill(0.0);
			padded_data.times.resize(padded_T);
			for (size_t t = 0; t < padded_T; ++t) {
				padded_data.times[t] = time0 + static_cast<double>(t);
			}
			unsigned int idx = T ? static_cast<unsigned int>(data.times.front() - time0) : 0;
			padded_data.probs.fill(1.0 / static_cast<double>(dim)); // default value
			input_to_padded.resize(T);
			const Eigen::MatrixXd& p = data.probs;
			const Eigen::VectorXd& ns = data.nbr_surveys;
			for (size_t t = 0; t < T; ++t) {
				assert(static_cast<size_t>(idx) < padded_T);
				padded_data.probs.col(idx) = p.col(t);
				padded_data.nbr_surveys[idx] = ns[t];
				input_to_padded[t] = idx; // map t-th data year to idx-th continuous year
				if (t + 1 < T) { // if we still have next year
					const int year_increment = static_cast<int>(times[t + 1] - times[t]); // how many years from this measured distribution to the next one
					if (year_increment < 1) {
						std::stringstream ss;
						ss << "Bad year number increment at index " << t;
						throw std::runtime_error(ss.str().c_str());
					}
					// pad missing year with old data and zero weight
					for (int t2 = 1; t2 < year_increment; ++t2) {
						++idx;
						padded_data.probs.col(idx) = p.col(t);
						padded_data.nbr_surveys[idx] = 0; // force the code to ignore this data
					}
				}
				++idx;
			}
			assert(idx <= padded_T);
			return padded_data;
		} else {
			// No need for padding
			return data;
		}
	}

	ObservedDiscreteData ObservedDiscreteData::pad(const ObservedDiscreteData& data, std::vector<size_t>& input_to_padded) {
		const size_t T = data.nbr_surveys.size();
		const double min_time = first_time(data);
		const double max_time = last_time(data);
		const size_t padded_T = static_cast<size_t>(max_time - min_time + 1); // T for a sample with missing years padded with dummy values
		if (padded_T < T) {
			std::stringstream ss;
			ss << "Wrong begin and end years";
			throw std::runtime_error(ss.str().c_str());
		}
		return pad_impl(data, input_to_padded, min_time, padded_T);
	}

	template <class V> static void divide_double_vector(V& vec, double x) {
		std::transform(vec.begin(), vec.end(), vec.begin(), [x](double v){return v / x; });
	}

	void ObservedDiscreteData::change_time_unit(ObservedDiscreteData& data, double new_time_unit) {
		divide_double_vector(data.times, new_time_unit);
		const auto rend = data.ltimes.row_end();
		for (auto it = data.ltimes.row_begin(); it != rend; ++it) {
			auto row = *it;
			divide_double_vector(row, new_time_unit);
		}
	}

	template <class Iter> double smallest_time_increment_impl(Iter i1, const Iter end) {
		double result = std::numeric_limits<double>::infinity();
		if (i1 != end) {
			Iter i2 = i1 + 1;
			if (i2 != end) {
				while (i2 != end) {
					const double incr = *i2 - *i1;
					result = std::min(result, incr);
					++i1;
					++i2;
				}
			}
		}
		return result;
	}

	template <class V> double smallest_time_increment_impl(const V& times) {
		return smallest_time_increment_impl(times.begin(), times.end());
	}

	double ObservedDiscreteData::smallest_time_increment(const std::vector<double>& times) {
		return smallest_time_increment_impl(times);
	}

	double ObservedDiscreteData::discretize_times(ObservedDiscreteData& data) {
		double dt = smallest_time_increment(data.times);
		const auto lrend = data.ltimes.row_end();
		for (auto lrit = data.ltimes.row_begin(); lrit != lrend; ++lrit) {
			const double row_dt = smallest_time_increment_impl(*lrit);
			dt = std::min(dt, row_dt);
		}
		change_time_unit(data, dt);
		return dt;
	}

	double ObservedDiscreteData::first_time(const ObservedDiscreteData& data) {
		double time0 = data.times.empty() ? std::numeric_limits<double>::infinity() : data.times.front();
		const auto lrend = data.ltimes.row_end();
		for (auto lrit = data.ltimes.row_begin(); lrit != lrend; ++lrit) {
			const auto row = *lrit;
			if (row.size()) {
				time0 = std::min(time0, row[0]);
			}
		}
		return time0;
	}

	double ObservedDiscreteData::last_time(const ObservedDiscreteData& data) {
		double max_time = data.times.empty() ? (-std::numeric_limits<double>::infinity()) : data.times.back();
		const auto lrend = data.ltimes.row_end();
		for (auto lrit = data.ltimes.row_begin(); lrit != lrend; ++lrit) {
			const auto row = *lrit;
			const size_t row_size = row.size();
			if (row_size) {
				max_time = std::max(max_time, row[row_size - 1]);
			}
		}
		return max_time;
	}

	unsigned int ObservedDiscreteData::dim(const ObservedDiscreteData& data) {
		if (data.probs.size()) {
			return static_cast<unsigned int>(data.probs.rows());
		} else {
			if (data.ltrajs.nbr_elements()) {
				const auto max_element_iter = std::max_element(data.ltrajs.flat_begin(), data.ltrajs.flat_end());
				assert(max_element_iter != data.ltrajs.flat_end());
				return static_cast<unsigned int>((*max_element_iter) + 1);
			} else {
				return 0;
			}
		}
	}

	

	typedef std::map<double, std::vector<size_t>> count_map_t;

    /* unused
       static std::vector<size_t>& get_counts(count_map_t& map, double time) {
       //auto it = map.find(time);

       }
    */

	static ObservedDiscreteData to_cross_sectional_impl(const ObservedDiscreteData& data) {
		const unsigned int dim = ObservedDiscreteData::dim(data);
		count_map_t counts;
		const size_t T = data.times.size();
		const auto get_counts = [&counts, dim](double time) ->std::vector<size_t>& {
			auto it = counts.find(time);
			if (it == counts.end()) {
				// insert new vector
				return counts.insert(std::make_pair(time, std::vector<size_t>(dim, 0))).first->second;
			} else {
				// return reference to already inserted vector
				return it->second;
			}
		};

		// Copy cross-sectional data
		for (size_t t = 0; t < T; ++t) {
			auto& counts_vec = get_counts(data.times[t]);
			const auto col = data.probs.col(t);
			const double n_t = data.nbr_surveys[t];
			for (size_t k = 0; k < dim; ++k) {
				counts_vec[k] += static_cast<size_t>(n_t * col[k]);
			}
		}
		// Collect from longitudinal data
		auto ltrajs_it = data.ltrajs.flat_begin();
		const auto ltrajs_end = data.ltrajs.flat_end();
		auto ltimes_it = data.ltimes.flat_begin();
		while (ltrajs_it != ltrajs_end) {
			++get_counts(*ltimes_it)[*ltrajs_it];
			++ltimes_it;
			++ltrajs_it;
		}
		// Convert back to probability distributions
		const unsigned int newT = static_cast<unsigned int>(counts.size());
		ObservedDiscreteData reduced(dim, newT);
		size_t idx = 0;
		for (auto it = counts.begin(); it != counts.end(); ++it) {
			reduced.times[idx] = it->first;
			const double n_t = static_cast<double>(std::accumulate(it->second.begin(), it->second.end(), static_cast<size_t>(0)));
			reduced.nbr_surveys[idx] = n_t;
			auto col = reduced.probs.col(idx);
			std::transform(it->second.begin(), it->second.end(), make_index_iterator_begin(col), [n_t](size_t c){return static_cast<double>(c) / n_t; });
			++idx;
		}
		return reduced;
	}

	ObservedDiscreteData ObservedDiscreteData::to_cross_sectional(const ObservedDiscreteData& data, double fract) {
		assert(fract >= 0.0);
		assert(fract <= 1.0);
		const size_t ntrajs = data.ltimes.size();
		const size_t ntrajs_reduced = static_cast<size_t>(static_cast<double>(ntrajs) * fract);
		if (ntrajs_reduced == 0) {
			return data;
		} else if (ntrajs_reduced == ntrajs) {
			return to_cross_sectional_impl(data);
		} else {
			std::mt19937 rng(37);
			std::vector<size_t> indices(ntrajs);
			std::iota(indices.begin(), indices.end(), 0);
			std::shuffle(indices.begin(), indices.end(), rng);
			ObservedDiscreteData data_for_reduction(data);
			data_for_reduction.ltimes = data.ltimes.row_range(0, ntrajs_reduced);
			data_for_reduction.ltrajs = data.ltrajs.row_range(0, ntrajs_reduced);
			ObservedDiscreteData reduced = to_cross_sectional_impl(data_for_reduction);
			assert(reduced.ltimes.size() == 0);
			reduced.ltimes = data.ltimes.row_range(ntrajs_reduced, ntrajs - ntrajs_reduced);
			reduced.ltrajs = data.ltrajs.row_range(ntrajs_reduced, ntrajs - ntrajs_reduced);
			return reduced;
		}
	}

	void ObservedDiscreteData::cross_sectional_to_stream(std::ostream& stream, const ObservedDiscreteData& data) {
		const size_t dim = ObservedDiscreteData::dim(data);
		const size_t T = data.times.size();
		stream.precision(16);
		for (size_t t = 0; t < T; ++t) {
			for (size_t k = 0; k < dim; ++k) {
				stream << data.probs(k, t) << "\t";
			}
			stream << data.nbr_surveys[t] << "\t" << data.times[t] << "\n";
		}
	}

    ObservedDiscreteData::index_t ObservedDiscreteData::load_trajectories(const char* fname, ObservedDiscreteData& data) {
        averisera::CSVFileReader reader(fname, CSV::Delimiter::TAB, CSV::QuoteCharacter::DOUBLE_QUOTE, true); // column names will be years
        const size_t ncol = reader.count_columns(); // number of columns in the file
        std::vector<double> all_times(ncol);
        const std::vector<std::string> col_names(reader.read_column_names());
        std::transform(col_names.begin(), col_names.end(), all_times.begin(), [](const std::string& name){return boost::lexical_cast<double>(name); });
        const ObservedDiscreteData::index_t nrow = reader.count_data_rows();
        reader.to_data();
        std::vector<double> data_row(ncol);
        std::vector<std::vector<ObservedDiscreteData::lcidx_t>> ltrajs(nrow);
        std::vector<std::vector<double>> ltimes(nrow);
        for (size_t r = 0; r < nrow; ++r) {				
            const size_t n_read = reader.read_data_row(data_row);
            auto& ltrajs_row = ltrajs[r];
            auto& ltimes_row = ltimes[r];
            ltrajs_row.reserve(ncol);
            ltimes_row.reserve(ncol);
            assert(ltrajs_row.empty());
            assert(ltimes_row.empty());
            for (size_t c = 0; c < n_read; ++c) {
                const double val = data_row[c];
                if (val >= 0) {
                    ltrajs_row.push_back(static_cast<ObservedDiscreteData::lcidx_t>(val));
                    ltimes_row.push_back(all_times[c]);
                }
            }
        }
        data.ltrajs = Jagged2DArray<ObservedDiscreteData::lcidx_t>(ltrajs);
        data.ltimes = Jagged2DArray<double>(ltimes);		
        return nrow;
    }

    std::pair<ObservedDiscreteData::lcidx_t, ObservedDiscreteData::index_t> ObservedDiscreteData::load_probabilities(const char* fname, ObservedDiscreteData& data, bool check_sum) {
        const auto dims = Utils::load_probabilities(fname, data.probs, check_sum, data.nbr_surveys, data.times);		
		return std::make_pair(MathUtils::safe_cast<lcidx_t>(dims.first), dims.second);
    }

	void ObservedDiscreteData::validate() const {
		check_equals<size_t, size_t, DataException>(ltimes.size(), ltrajs.size(), "Number of time and value vectors must be equal");
		auto values_row_iter = ltrajs.row_begin();
		size_t row_index = 0;
		for (auto times_row_iter = ltimes.row_begin(); times_row_iter != ltimes.row_end(); ++times_row_iter, ++values_row_iter, ++row_index) {
			assert(values_row_iter != ltrajs.row_end());
			const auto times_row_ref = *times_row_iter;
			const auto values_row_ref = *values_row_iter;
			check_equals<size_t, size_t, DataException>(times_row_ref.size(), values_row_ref.size(), (boost::format("Trajectory row %d: times vector has length %d but values vector has length %d") % row_index % times_row_ref.size() % values_row_ref.size()).str().c_str());
			check_greater<size_t, size_t, DataException>(times_row_ref.size(), 0, "Trajectory must have non-zero length");
			if (times_row_ref.size() > 1) {
				for (size_t i = 1; i < times_row_ref.size(); ++i) {
					check_greater(times_row_ref[i], times_row_ref[i - 1], "Times must be strictly increasing");
				}
			}
		}
	}
}

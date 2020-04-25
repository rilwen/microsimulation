#include "observer_stats.hpp"
#include "../contexts.hpp"
#include "../immutable_context.hpp"
#include "../predicate.hpp"
#include "../person.hpp"
#include "../population.hpp"
#include "core/statistics.hpp"
#include <algorithm>
#include <cassert>
#include <iterator>

namespace averisera {
    namespace microsim {
        template <class T, class V> ObserverStats<T, V>::ObserverStats(std::shared_ptr<ObserverResultSaver> result_saver, const std::vector<ObservedQuantity<T>>& variables, std::shared_ptr<const Predicate<T>> predicate, bool calc_medians, unsigned int precision)
            : Observer(result_saver), _variables(variables), _predicate(predicate), _precision(precision), calc_medians_(calc_medians) {
            if (!predicate) {
                throw std::domain_error("ObserverStats: null predicate");
            }            
        }

        template <class T> static std::vector<ObservedQuantity<T>> to_functions(const std::vector<std::string>& variables) {
            std::vector<ObservedQuantity<T>> functions;
            functions.reserve(variables.size());
            std::transform(variables.begin(), variables.end(), std::back_inserter(functions), [](const std::string& name) {
                    return ObservedQuantity<T>::last_as_double(name);
                });
            return std::move(functions);
        }

        template <class T, class V> ObserverStats<T, V>::ObserverStats(std::shared_ptr<ObserverResultSaver> result_saver, const std::vector<std::string>& variables, std::shared_ptr<const Predicate<T>> predicate, bool calc_medians, unsigned int precision)
            : ObserverStats(result_saver, to_functions<T>(variables), predicate, calc_medians, precision) {
        }
            
        template <class T, class V> void ObserverStats<T, V>::observe(const Population& population, const Contexts& ctx) {
            const Date asof = ctx.asof();            
			auto stats_it = _stats.find(asof);
            if (stats_it == _stats.end()) {
				stats_it = _stats.insert(std::make_pair(asof, RunningStatisticsMulti<V>(_variables.size()))).first;
            }
			auto medians_it = medians_.find(asof);
			if (calc_medians_ && (medians_it == medians_.end())) {
				medians_it = medians_.insert(std::make_pair(asof, std::vector<V>(_variables.size()))).first;
			}
			RunningStatisticsMulti<V>& stats = stats_it->second;
            const std::vector<std::shared_ptr<T>>& members = population.members<T>();            
			std::vector<std::shared_ptr<T>> selected_members;
			for (auto sit = members.begin(); sit != members.end(); ++sit) {
				if (_predicate->select(**sit, ctx)) {
					selected_members.push_back(*sit);
				}
			}
			selected_members.shrink_to_fit();
			const size_t N = selected_members.size();
			std::vector<V> values(stats.dim());
			std::vector<std::vector<V>> all_values;
			if (calc_medians_) {
				all_values.resize(stats.dim());
				std::for_each(all_values.begin(), all_values.end(), [N](std::vector<V>& v) { v.reserve(N); });
			}
            for (auto sit = selected_members.begin(); sit != selected_members.end(); ++sit) {
                auto val_it = values.begin();
				auto all_val_it = all_values.begin();
                for (auto vit = _variables.begin(); vit != _variables.end(); ++vit, ++val_it) {
                    assert(val_it != values.end());
					const V x = (*vit)(**sit, ctx);
					*val_it = x;
					if (calc_medians_) {
						// save value for median calculation
						all_val_it->push_back(x);
						++all_val_it;
					}
                }
				val_it = values.begin();
				for (size_t idx = 0; idx < stats.dim(); ++idx, ++val_it) {
					assert(val_it != values.end());
					const V x = *val_it;
					stats.marginal(idx).add_if_not_nan(x);
					auto val_it2 = values.begin();
					for (size_t idx2 = 0; idx2 < idx; ++idx2, ++val_it2) {
						assert(val_it2 != val_it);
						stats.covariance(idx, idx2).add_if_not_nan(x, *val_it2);
					}
				}
            }
			if (calc_medians_) {
				std::vector<V>& medians = medians_it->second;
				auto dst_it = medians.begin();
				for (auto src_it = all_values.begin(); src_it != all_values.end(); ++src_it, ++dst_it) {
					*dst_it = Statistics::median(*src_it);
				}
			}
        }
        
        template <class T, class V> void ObserverStats<T, V>::save_results(std::ostream& os, const ImmutableContext&) const {
			//const Schedule& sim_schedule = im_ctx.schedule();
			const auto old_precision = os.precision();
			os.precision(_precision);
			os << "ObserverStats\n";
			std::vector<Date> dates;
			dates.reserve(_stats.size());
			for (const auto& kv : _stats) {
				dates.push_back(kv.first);
			}
			std::sort(dates.begin(), dates.end());
			std::vector<V> nans(_variables.size(), std::numeric_limits<V>::quiet_NaN());
			for (auto d: dates) {
				const auto mit = _stats.find(d);
				const auto medians_it = medians_.find(d);
				assert(mit != _stats.end());
                os << "#MARGINALS " << mit->first << "\n";
				os << "name\tmean\tstd.dev.\tskew\tkurt.\tmin\tmax\tmedian\n";
				const RunningStatisticsMulti<V>& sv = mit->second;
                assert(sv.dim() == _variables.size());
				const std::vector<V>& medians = calc_medians_ ? medians_it->second : nans;
                size_t idx = 0;
                for (auto varit = _variables.begin(); varit != _variables.end(); ++idx, ++varit) {
					assert(idx < sv.dim());
					const RunningStatistics<V>& rs = sv.marginal(idx);
					os << varit->name() << "\t" << rs.mean() << "\t" << rs.standard_deviation() << "\t" << rs.skewness() << "\t" << rs.kurtosis() << "\t" << rs.min() << "\t" << rs.max() << "\t" << medians[idx];
                    os << "\n";
                }

                os << "#COVARIANCES " << mit->first << "\n";
                for (auto varit = _variables.begin(); varit != _variables.end(); ++varit) {
                    os << "\t" << varit->name();                    
                }
                os << "\n";
				idx = 0;
				for (auto varit = _variables.begin(); varit != _variables.end(); ++varit, ++idx) {
					assert(idx < sv.dim());
					os << varit->name();
					for (size_t j = 0; j < idx; ++j) {
						os << "\t" << sv.covariance(idx, j).covariance();
					}
					os << "\t" << sv.marginal(idx).variance() << "\n";
				}

				os << "#CORRELATIONS " << mit->first << "\n";
				for (auto varit = _variables.begin(); varit != _variables.end(); ++varit) {
					os << "\t" << varit->name();
				}
				os << "\n";
				idx = 0;
				for (auto varit = _variables.begin(); varit != _variables.end(); ++varit, ++idx) {
					assert(idx < sv.dim());
					os << varit->name();
					for (size_t j = 0; j < idx; ++j) {
						os << "\t" << sv.covariance(idx, j).correlation();
					}
					os << "\t1.0\n";
				}
            }
			save_single_result(os, [](const RunningStatistics<V>& rs) { return rs.mean(); }, "MEAN", dates);
			save_single_result(os, [](const RunningStatistics<V>& rs) { return rs.mean() * static_cast<double>(rs.nbr_samples()); }, "SUM", dates);
			save_single_result(os, [](const RunningStatistics<V>& rs) { return rs.standard_deviation(); }, "ST_DEV", dates);
			save_single_result(os, medians_, "MEDIAN", dates);
			os.precision(old_precision);
        }

		template <class T, class V> template <class Functor> void ObserverStats<T, V>::save_single_result(std::ostream& os, Functor f, const char* result_name, const std::vector<Date>& dates) const {
			os << "#" << result_name << "\n";
			os << "Date";
			for (auto varit = _variables.begin(); varit != _variables.end(); ++varit) {
				os << "\t" << varit->name();
			}
			os << "\n";
			for (auto d : dates) {
				const auto mit = _stats.find(d);
				assert(mit != _stats.end());
				os << d;
				const RunningStatisticsMulti<V>& sv = mit->second;
				for (size_t idx = 0; idx < sv.dim(); ++idx) {
					const RunningStatistics<V>& rs = sv.marginal(idx);
					os << "\t" << f(rs);
				}
				os << "\n";
			}
		}

		template <class T, class V> void ObserverStats<T, V>::save_single_result(std::ostream& os, const std::unordered_map<Date, std::vector<V>>& results, const char* result_name, const std::vector<Date>& dates) const {
			os << "#" << result_name << "\n";
			os << "Date";
			for (auto varit = _variables.begin(); varit != _variables.end(); ++varit) {
				os << "\t" << varit->name();
			}
			os << "\n";
			for (auto d : dates) {
				const auto rit = results.find(d);
				assert(rit != results.end());
				os << d;
				const auto& v = rit->second;
				for (const V& x: v) {
					os << "\t" << x;
				}
				os << "\n";
			}
		}

        template class ObserverStats<Person>;
    }
}

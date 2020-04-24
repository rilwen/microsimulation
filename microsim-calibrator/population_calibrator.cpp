#include "population_calibrator.hpp"
#include "rate_calibrator.hpp"
#include "core/data_exception.hpp"
#include "core/generic_distribution_enumerated.hpp"
#include "core/inclusion.hpp"
#include "core/log.hpp"
#include "core/preconditions.hpp"
#include "microsim-core/person_attributes.hpp"
#include "microsim-core/person_attributes_distribution.hpp"
#include "microsim-simulator/initialiser/generation.hpp"
#include <algorithm>
#include <boost/format.hpp>

namespace averisera {
	namespace microsim {
		namespace PopulationCalibrator {
			pop_data_type load_population_numbers(CSVFileReader& reader, const Ethnicity::IndexConversions& ic) {
				DataFrame<Ethnicity::index_range_type, age_group_type> result(DataFrame<Ethnicity::index_range_type, age_group_type>::from_csv_file(reader, [&ic](const std::string& str) { return ic.index_range_from_string(str); }, RateCalibrator::age_group_from_string, false, false));
				check_that<DataException>(std::all_of(result.values_begin(), result.values_end(), [](const double& x) {
					return x >= 0;
				}), "PopulationCalibrator: population numbers must be non-negative");
				check_that<DataException>(result.nbr_cols() > 0, "PopulationCalibrator: at least one ethnic group range required");
				check_that<DataException>(result.nbr_rows() > 0, "PopulationCalibrator: at least one age group required");
				check_equals<age_type, age_type, DataException>(0u, result.index().front().begin(), "PopulationCalibrator: age ranges must start at 0");
				LOG_TRACE() << "PopulationCalibrator::load_population_numbers: loaded data from file " << reader.file_name() << ":\n" << result;
				result.sort_index();
				result.sort_columns();
				check_that<DataException>(Ethnicity::index_range_type::all_disjoint(result.columns()), "PopulationCalibrator: ethnic group ranges must be disjoint");
				check_that<DataException>(age_group_type::all_adjacent(result.index()), "PopulationCalibrator: age groups must be adjacent");
				pop_data_type result2(result.values(), ic.index_ranges_to_sets(result.columns()), result.index());
				LOG_DEBUG() << "PopulationCalibrator: loaded population numbers from file " << reader.file_name() << ":\n" << result2;
				return result2;
			}

			static size_t find_most_granular(const std::vector<pop_data_type>& dfs) {
				assert(!dfs.empty());
				const size_t n = dfs.size();
				size_t result = 0;
				for (size_t i = 1; i < n; ++i) {
					const InclusionRelation rel = Inclusion::disjoint_elements_inclusion(dfs[result].columns(), dfs[i].columns());
					if (InclusionRelation::CONTAINS == rel) {
						// i has finer resolution
						result = i;
					} else if (InclusionRelation::IS_CONTAINED_BY != rel && InclusionRelation::EQUALS != rel) {
						// incompatible ranges
						throw DataException(boost::str(boost::format("PopulationCalibrator: incompatible ethnic group ranges: %s (%d) vs %s (%d), relation == %d") % dfs[result].columns() % result % dfs[i].columns() % i % static_cast<int>(rel)));
					}
				}
				return result;
			}

			
			static void copy_ethnic_group_ranges(const pop_data_type& src, pop_data_type& dst) {
				check_equals(src.index(), dst.index(), "PopulationCalibrator: indices must be equal");
				pop_data_type new_dst(src.columns(), src.index());
				std::vector<Ethnicity::index_set_type> contained_labels;
				std::vector<double> weights;
				std::vector<pop_data_type::size_type> col_indices;
				for (const Ethnicity::index_set_type& dr : dst.columns()) {
					contained_labels.clear();
					std::copy_if(src.columns().begin(), src.columns().end(), std::back_inserter(contained_labels), [&dr](const Ethnicity::index_set_type& sr) { return Inclusion::contains(dr, sr); });
					const size_t n = contained_labels.size();
					assert(n);
					weights.resize(n);			
					col_indices.clear();
					col_indices.reserve(n);
					for (const auto& cl : contained_labels) {
						col_indices.push_back(src.col_idx(cl));
					}
					const auto cd = dst.col_idx(dr);
					for (auto cs: col_indices) {
						new_dst.col_values_ix(cs) = dst.col_values_ix(cd);
					}					
					for (pop_data_type::size_type r = 0; r < src.nbr_rows(); ++r) {
						double sum = 0;
						auto w_it = weights.begin();
						for (auto cs : col_indices) {
							const double w = std::abs(src.ix(r, cs));
							assert(w >= 0);
							*w_it = w;							
							sum += w;
							++w_it;
						}
						if (sum > 0) {
							std::transform(weights.begin(), weights.end(), weights.begin(), [sum](double w) { return w / sum; });
						} else {
							std::fill(weights.begin(), weights.end(), 1.0 / static_cast<double>(n));
						}
						w_it = weights.begin();
						for (auto cs : col_indices) {
							new_dst.ix(r, cs) *= *w_it;
							++w_it;
						}
					}
				}		
				dst.swap(new_dst);
			}

			void sync_ethnic_group_ranges(std::vector<pop_data_type>& dfs) {
				check_that(!dfs.empty());
				const size_t k = find_most_granular(dfs);
				const size_t n = dfs.size();
				for (size_t i = k + 1; i < n; ++i) {
					copy_ethnic_group_ranges(dfs[i - 1], dfs[i]);
				}
				for (size_t i = k; i > 0; --i) {
					copy_ethnic_group_ranges(dfs[i], dfs[i - 1]);
				}
			}

			std::vector<pop_data_type> load_population_numbers(const std::vector<std::string>& file_names, CSV::Delimiter delim, const Ethnicity::IndexConversions& ic) {
				std::vector<pop_data_type> result;
				result.reserve(file_names.size());
				for (const std::string& file_name : file_names) {
					CSVFileReader reader(file_name, delim);
					result.push_back(load_population_numbers(reader, ic));
				}
				sync_ethnic_group_ranges(result);
				return result;
			}

			std::vector<Generation> make_generations(const pop_data_type& females, const pop_data_type& males, const int year, const age_type max_age, const Ethnicity::IndexConversions& ic, const Date sim_start) {
				check_that<DataException>(females.index() == males.index(), "PopulationCalibrator: female and male data need the same age group structure");
				const auto& age_groups = females.index();
				const auto& ethn_sets = females.columns();
				std::vector<Generation> generations;
				const size_t nr = age_groups.size();
				generations.reserve(nr);
				const double male_population = males.values().array().sum();
				const double female_population = females.values().array().sum();
				const double total_population = female_population + male_population;
				check_that<DataException>(total_population > 0, "PopulationCalibrator: total population must be positive");
				const size_t nbr_pa_values = 2 * ic.size();
				std::vector<PersonAttributes> pa_values;				
				std::vector<double> pa_probs(nbr_pa_values);
				const std::array<std::pair<Sex, const pop_data_type*>, 2> gender_data({ std::make_pair(Sex::FEMALE, &females),
					std::make_pair(Sex::MALE, &males) });
				pa_values.reserve(nbr_pa_values);
				for (const auto& gd : gender_data) {
					const Sex sex = gd.first;
					for (const auto& cl : ethn_sets) {
						for (Ethnicity::group_index_type gidx : cl) {
							pa_values.push_back(PersonAttributes(sex, gidx));
						}
					}
				}
				for (size_t r = 0; r < nr; ++r) {
					const auto& age_group = age_groups[r];
					if (age_group.begin() > max_age) {
						throw DataException(boost::str(boost::format("ProbabilityCalibrator: age %d beyond the maximum value %d") % age_group.begin() % max_age));
					}
					// assume age_group is [begin, end) and that the census was done at the end of year
					const Date begin(static_cast<Date::year_type>(year - std::min(max_age, age_group.end()) + 1), 1, 1);
					Date end(static_cast<Date::year_type>(year - age_group.begin() + 1), 1, 1);
					end = std::min(end, sim_start);
					const double total_population_group = females.row_values_ix(r).sum() + males.row_values_ix(r).sum();
					const double age_group_prob = total_population_group / total_population;
					auto pa_probs_it = pa_probs.begin();
					for (const auto& gd : gender_data) {
						const pop_data_type& data = *gd.second;
						for (const auto& cl : ethn_sets) {
							const auto col = data.col_values(cl);
							const double p = col[r] / total_population_group / static_cast<double>(cl.size());
                            std::for_each(cl.begin(), cl.end(), [p, &pa_probs_it](const auto&) {
                                    *pa_probs_it = p;
                                    ++pa_probs_it;
                                });
						}
					}
					assert(pa_probs_it == pa_probs.end());
					generations.push_back(Generation(begin, end, PersonAttributesDistribution(GenericDistributionEnumerated<PersonAttributes>::from_unsorted(pa_values, pa_probs)), age_group_prob));
				}
				return generations;
			}

			static void sync_ethnic_group_ranges(std::vector<pop_data_type>& dfs1, std::vector<pop_data_type>& dfs2) {
				if (!dfs1.empty() && !dfs2.empty()) {
					std::vector<pop_data_type> tmp(2);
					tmp[0] = std::move(dfs1.front());
					tmp[1] = std::move(dfs2.front());
					sync_ethnic_group_ranges(tmp);
					dfs1.front() = std::move(tmp[0]);
					dfs2.front() = std::move(tmp[1]);
				}
				sync_ethnic_group_ranges(dfs1);
				sync_ethnic_group_ranges(dfs2);
			}

			std::vector<pop_data_type> subtract_population_numbers(std::vector<pop_data_type> a, std::vector<pop_data_type> b) {
				const size_t n = a.size();
				check_equals(n, b.size());
				if (!n) {
					return a;
				}
				sync_ethnic_group_ranges(a, b);
				std::vector<pop_data_type> result;
				result.reserve(n);
				for (size_t i = 0; i < n; ++i) {
					result.push_back(a[i] - b[i]);
				}
				return result;
			}

			std::vector<pop_data_type> calc_population_increments(const std::vector<pop_data_type>& numbers) {
				const size_t n = numbers.size();
				check_that(n > 0);
				std::vector<pop_data_type> result;
				result.reserve(n - 1);
				for (size_t i = 1; i < n; ++i) {
					result.push_back(numbers[i] - numbers[i - 1]);
				}
				return result;
			}
		}
	}
}

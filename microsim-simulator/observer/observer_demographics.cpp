// (C) Averisera Ltd 2014-2020
#include "observer_demographics.hpp"
#include "microsim-core/schedule.hpp"
#include "../contexts.hpp"
#include "../immutable_context.hpp"
#include "../person.hpp"
#include "../person_data.hpp"
#include "../population.hpp"
#include "core/inclusion.hpp"
#include "core/preconditions.hpp"
#include <cassert>
#include <fstream>

namespace averisera {
	namespace microsim {
		ObserverDemographics::ObserverDemographics(std::shared_ptr<ObserverResultSaver> result_saver, const std::string& category, const age_ranges_type& age_ranges, size_t nbr_dates, const std::string& own_filename_stub)
			: Observer(result_saver), category_(category), age_ranges_(age_ranges), _nbr_dates(nbr_dates), own_filename_stub_(own_filename_stub), init_counters_(_nbr_dates, 0) {
			check_that(Inclusion::all_disjoint<false>(age_ranges), "ObserverDemographics: age ranges are not disjoint");
		}

		void ObserverDemographics::observe_persons(const std::vector<std::shared_ptr<Person>>& persons, const Contexts& ctx) {
			const size_t idx = ctx.asof_idx();
			if (idx < _nbr_dates) {
				const Date asof = ctx.asof();
				if (idx > 0) {
					const Schedule& schedule = ctx.immutable_ctx().schedule();
					const Date prev_asof = schedule.date(idx - 1);
					size_t cnt_newborns = 0;
					for (const Person::shared_ptr& person_ptr : persons) {
						const PersonAttributes& attribs = person_ptr->attributes();
						const Date dob = person_ptr->date_of_birth();
						if (dob >= asof) {
							// newborn in the future, will be counted in next step of simulation
							++cnt_newborns;
							continue;
						}
						const double age = person_ptr->age_fract(asof);
						if (person_ptr->is_alive(asof)) {
							++get_counters(_pop_counters, attribs, age)[idx];
						}							

						if (dob >= prev_asof && dob < asof) {
							// it's a newborn person
							// find mother if possible
							double age_at_birth = 0.0; // mother's age if we can find it, child's age otherwise (==0)
							if (const auto mother_ptr = person_ptr->mother().lock()) {
								age_at_birth = mother_ptr->age_fract(dob);
								const Date id = mother_ptr->immigration_date();

								if (!id.is_not_a_date()) {
									//// HACK HACK HACK
									//// For Brexit paper
									//static const Date min_date(2016, 7, 1);
									//static const Date max_date(2036, 7, 1);
									//const bool pass = dob >= min_date && dob < max_date && id >= min_date && id < max_date;

									//if (pass) { // replace with true if removing HACK
										// child's mother is an immigrant
										++get_counters(birth_to_immigrants_by_dob_counters_, attribs, age_at_birth)[idx];
										const auto im_idx = schedule.find_containing_period(id) + 1;
										assert(im_idx < _nbr_dates);
										++get_counters(birth_to_immigrants_by_id_counters_, attribs, age_at_birth)[im_idx];
									//}
								}
							} else {
								LOG_WARN() << "ObserverDemographics(" << category_ << "): could not find mother of a newborn person: " << person_ptr->to_data(ctx.immutable_ctx());								
							}
							++get_counters(_birth_counters, attribs, age_at_birth)[idx];
						}
						const Date dod = person_ptr->date_of_death();
						if (!dod.is_not_a_date()) {
							if (dod >= prev_asof && dod < asof) {
								++get_counters(_death_counters, attribs, age)[idx];
							}
						}
					}
					LOG_TRACE() << "ObserverDemographics(" << category_ << "): skipped " << cnt_newborns << " persons born in the future as of " << asof;
				} else {
					for (const Person::shared_ptr& person_ptr : persons) {
						const PersonAttributes& attribs = person_ptr->attributes();
						if (person_ptr->is_alive(asof)) {
							const double age = person_ptr->age_fract(asof);
							++get_counters(_pop_counters, attribs, age)[idx];
						}
					}
				}
			}
		}

		void ObserverDemographics::save_results(std::ostream& os, const ImmutableContext& im_ctx) const {
			os << "ObserverDemographics_" << category_ << "\n";
			const Schedule& sim_schedule = im_ctx.schedule();
			os << "Population\n";
			save_event_stats(os, sim_schedule, _pop_counters, im_ctx, "population.csv", false);
			os << "Births\n";
			save_event_stats(os, sim_schedule, _birth_counters, im_ctx, "births.csv", true);
			os << "BirthsToImmigrantsByDOB\n";
			save_event_stats(os, sim_schedule, birth_to_immigrants_by_dob_counters_, im_ctx, "births2immigrants_dob.csv", true);
			os << "BirthsToImmigrantsByID\n";
			save_event_stats(os, sim_schedule, birth_to_immigrants_by_id_counters_, im_ctx, "births2immigrants_id.csv", true);
			os << "Deaths\n";
			save_event_stats(os, sim_schedule, _death_counters, im_ctx, "deaths.csv", true);
		}

		const ObserverDemographics::age_range_type& ObserverDemographics::get_age_range(double age) const {
			const int iage = static_cast<age_range_type::value_type>(age);
			for (auto it = age_ranges_.begin(); it != age_ranges_.end(); ++it) {
				if (it->contains(iage)) {
					return *it;
				}
			}
			throw std::runtime_error(boost::str(boost::format("ObserverDemographics: no age range contains age %g") % age));
		}

		ObserverDemographics::counters_type& ObserverDemographics::get_counters(CountersMaps& maps, PersonAttributes attribs, double age) {
			counters_map_type& map = attribs.sex() == Sex::FEMALE ? maps.female : maps.male;
			const key_type key(get_age_range(age), attribs.ethnicity());
			counters_map_type::iterator it = map.find(key);
			if (it == map.end()) {
				it = map.insert(std::make_pair(key, init_counters())).first;
			}
			return it->second;
		}

		const ObserverDemographics::counters_type& ObserverDemographics::get_counters(const CountersMaps& maps, PersonAttributes attribs, double age) const {
			const counters_map_type& map = attribs.sex() == Sex::FEMALE ? maps.female : maps.male;
			const key_type key(get_age_range(age), attribs.ethnicity());
			return StlUtils::get(map, key, init_counters_);
		}

		void ObserverDemographics::save_event_stats(std::ostream& ext_os, const Schedule& sim_schedule, const CountersMaps& maps, const ImmutableContext& im_ctx, const std::string& suffix, const bool deltas) const {
			save_event_stats(ext_os, sim_schedule, maps.female, im_ctx, std::string("female_") + suffix, deltas);
			save_event_stats(ext_os, sim_schedule, maps.male, im_ctx, std::string("male_") + suffix, deltas);
			save_event_stats(ext_os, sim_schedule, maps.get_both_sexes(), im_ctx, std::string("both_") + suffix, deltas);
		}

		void ObserverDemographics::save_event_stats(std::ostream& ext_os, const Schedule& sim_schedule, const counters_map_type& map, const ImmutableContext& im_ctx, const std::string& suffix, const bool deltas) const {
			std::unique_ptr<std::ofstream> outf;
			if (!own_filename_stub_.empty()) {
				const std::string filename(own_filename_stub_ + category_ + "_" + suffix);
				LOG_DEBUG() << "ObserverDemographics: writing data to own file " << filename;
				outf = std::make_unique<std::ofstream>(filename, std::ios_base::out);
			}
			std::ostream& os = outf ? *outf : ext_os;
			const auto print_time_label = [deltas, &os]() {
				if (deltas) {
					os << "BeginDate\tEndDate";
				} else {
					os << "Date";
				}
			};
			print_time_label();
			std::vector<key_type> keys;
			const auto& ic = im_ctx.ethnicity_conversions();
			keys.reserve(age_ranges_.size() * ic.size());
			for (const auto& age_range : age_ranges_) {
				for (Ethnicity::group_index_type eidx = 0; eidx < ic.size(); ++eidx) {
					keys.push_back(std::make_pair(age_range, eidx));
				}
			}
			std::sort(keys.begin(), keys.end());
			for (const auto& key : keys) {
				os << "\t" << key.first << "," << ic.name(key.second);
			}
			os << "\n";
			const size_t first_idx = deltas ? 1 : 0;
			const auto print_time_value = [deltas, &sim_schedule, &os](size_t i) {
				if (deltas) {
					os << sim_schedule.date(i - 1) << "\t" << sim_schedule.date(i);
				} else {
					os << sim_schedule.date(i);
				}
			};
			std::unordered_map<age_range_type, counters_type> by_age;
			std::unordered_map<PersonAttributes::ethnicity_t, counters_type> by_group;
			for (size_t i = first_idx; i < _nbr_dates; ++i) {
				print_time_value(i);
				for (const auto& key : keys) {
					const auto val_it = map.find(key);
					counter_type value = 0;
					if (val_it != map.end()) {
						value = val_it->second[i];
					}
					StlUtils::get(by_age, key.first, init_counters_)[i] += value;
					StlUtils::get(by_group, key.second, init_counters_)[i] += value;
					os << "\t" << value;
				}
				os << "\n";
			}
			os << "Grouped By Age\n";
			print_time_label();
			for (const auto& age_range : age_ranges_) {
				os << "\t" << age_range;
			}
			os << "\n";
			for (size_t i = first_idx; i < _nbr_dates; ++i) {
				print_time_value(i);
				for (const auto& age_range : age_ranges_) {
					os << "\t" << by_age[age_range][i];
				}
				os << "\n";
			}
			os << "Grouped By Ethnicity\n";
			print_time_label();
			for (Ethnicity::group_index_type eidx = 0; eidx < ic.size(); ++eidx) {
				os << "\t" << ic.name(eidx);
			}
			os << "\n";
			for (size_t i = first_idx; i < _nbr_dates; ++i) {
				print_time_value(i);
				for (Ethnicity::group_index_type eidx = 0; eidx < ic.size(); ++eidx) {
					os << "\t" << by_group[eidx][i];
				}
				os << "\n";
			}
		}

		static ObserverDemographics::counters_map_type sub(const ObserverDemographics::counters_map_type& l, const ObserverDemographics::counters_map_type& r) {
			check_equals(l.size(), r.size());
			ObserverDemographics::counters_map_type result;
			for (const auto& lkv : l) {
				const auto rit = r.find(lkv.first);
				check_that(rit != r.end());
				const auto& rv = rit->second;
				const size_t n = lkv.second.size();
				check_equals(n, rv.size());
				auto& v = result[lkv.first];
				for (size_t i = 0; i < n; ++i) {
					v[i] = lkv.second[i] - rv[i];
				}
			}
			return result;
		}

		ObserverDemographics::CountersMaps ObserverDemographics::CountersMaps::operator-(const CountersMaps& other) const {
			CountersMaps result;
			result.female = sub(female, other.female);
			result.male = sub(male, other.male);
			return result;
		}

		ObserverDemographics::counters_map_type ObserverDemographics::CountersMaps::get_both_sexes() const {
			counters_map_type both(female);
			for (const auto& kv : male) {
				const auto it = both.find(kv.first);
				if (it == both.end()) {
					both.insert(kv);
				} else {
					assert(kv.second.size() == it->second.size());
					std::transform(kv.second.begin(), kv.second.end(), it->second.begin(), it->second.begin(), [](counter_type c1, counter_type c2) { return c1 + c2; });
				}				
			}
			return both;
		}
	}
}

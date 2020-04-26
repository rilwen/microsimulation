// (C) Averisera Ltd 2014-2020
#include "migration_calibrator.hpp"
#include "rate_calibrator.hpp"
#include "microsim-core/hazard_curve.hpp"
#include "microsim-core/migration_model.hpp"
#include "microsim-simulator/migration/migration_generator_model.hpp"
#include "microsim-simulator/migration/migration_generator_return.hpp"
#include "microsim-simulator/person.hpp"
#include "microsim-simulator/predicate_factory.hpp"
#include "core/csv_file_reader.hpp"
#include "core/log.hpp"
#include <boost/format.hpp>

namespace averisera {
	namespace microsim {
		namespace MigrationCalibrator {
			const std::string& BASE_COL() {
				static const std::string str("BASE");
				return str;
			}
			
			const std::string& MIGRATION_COL() {
				static const std::string str("MIGRATION");
				return str;
			}

			const std::string& MODEL_COL() {
				static const std::string str("MODEL");
				return str;
			}

			static inline Date::month_type get_month(bool mid_year) {
				return mid_year ? 7 : 1;
			}

			Date make_migration_date(int year, bool mid_year) {
				return Date(static_cast<Date::year_type>(year), get_month(mid_year), 1);
			}

			DataFrame<std::string, Ethnicity::index_set_type> load_net_migration(CSVFileReader& reader, const Ethnicity::IndexConversions& ic) {
				typedef DataFrame<std::string, Ethnicity::index_range_type> df_type;
				df_type result = df_type::from_csv_file(reader, reader.default_converter_complaining<std::string>(0), [&ic](const std::string& str) { return ic.index_range_from_string(str.c_str()); }, false, false);
				if (result.nbr_cols() < 2) {
					throw DataException(boost::str(boost::format("MigrationCalibrator: expected at least 2 columns in file %s") % reader.file_name()));
				}
				if (!result.has_col_label(BASE_COL())) {
					throw DataException(boost::str(boost::format("MigrationCalibrator: expected column %s in file %s") % BASE_COL() % reader.file_name()));
				}
				if (!result.has_col_label(MIGRATION_COL())) {
					throw DataException(boost::str(boost::format("MigrationCalibrator: expected column %s in file %s") % MIGRATION_COL() % reader.file_name()));
				}
				LOG_DEBUG() << "MigrationCalibrator: Loaded net migration from " << reader.file_name() << ":\n" << result;
				return DataFrame<std::string, Ethnicity::index_set_type>(result.values(), result.columns(), ic.index_ranges_to_sets(result.index()));
			}

			static const std::string MODEL_RELATIVE = "RELATIVE";
			static const std::string MODEL_ABSOLUTE = "ABSOLUTE";

			static bool is_model_relative(const std::string& model_type) {
				if (model_type == MODEL_RELATIVE) {
					return true;
				} else if (model_type == MODEL_ABSOLUTE) {
					return false;
				} else {
					throw DataException(boost::str(boost::format("MigrationCalibrator: unknown model type %s") % model_type));
				}
			}

			static void add_migration_models(const MigrationDataSet& dataset, const NumericalRange<int>& target_year_range, const bool mid_year, std::vector<pred_model_pair>& result, const std::shared_ptr<const Predicate<Person>>& other_pred) {
				if (other_pred) {
					LOG_DEBUG() << "MigrationCalibrator::add_migration_models: other_pred=" << other_pred->as_string();
				}
				check_equals(dataset.data.nbr_rows(), dataset.types.size());
				const auto& data = dataset.data;
				const size_t nmodels = data.nbr_rows();
				const auto& model_types = dataset.types;
				assert(model_types.size() == nmodels);
				result.reserve(result.size() + nmodels);
				const Date::month_type month = get_month(mid_year);
				const auto& data_year_range = dataset.data_year_range;
				const Date data_dt_begin(static_cast<Date::year_type>(data_year_range.begin()), month, 1);
				const Date data_dt_end(static_cast<Date::year_type>(data_year_range.end()), month, 1);
				const Date target_dt_begin(static_cast<Date::year_type>(target_year_range.begin()), month, 1);
				const Date target_dt_end(static_cast<Date::year_type>(target_year_range.end()), month, 1);
				const std::shared_ptr<const Predicate<Person>> pred_asof(PredicateFactory::make_asof<Person>(target_dt_begin, target_dt_end));
				const auto base_col = data.col_values(BASE_COL());
				const auto migr_col = data.col_values(MIGRATION_COL());
				for (size_t r = 0; r < nmodels; ++r) {
					const auto& ethn_rng = data.index()[r]; // range of ethnic groups for which the model will work
					const std::shared_ptr<const Predicate<Person>> pred_ethn_rng(PredicateFactory::make_ethnicity(ethn_rng, true));
					std::unique_ptr<const Predicate<Person>> pred;
					if (other_pred) {
						pred = PredicateFactory::make_and({ pred_asof, pred_ethn_rng, other_pred });
					} else {
						pred = PredicateFactory::make_and({ pred_asof, pred_ethn_rng });
					}
					bool imr = is_model_relative(model_types[r]);
					const double base = base_col[r];
					const double migration = migr_col[r];
					LOG_TRACE() << "MigrationCalibrator: calibrating migration model for predicate " << pred->as_string() << ", BASE=" << base << ", MIGRATION=" << migration << " and TYPE=" << model_types[r]; 
					if (!(migration >= -base)) {
						throw DataException(boost::str(boost::format("MigrationCalibrator: number of emigrants %g cannot exceed initial size %g") % migration % base));
					}					
					const double dt = MigrationModel::calc_dt(data_dt_begin, data_dt_end);
					const double absolute_rate = imr ? 0.0 : migration / dt;
					MigrationModel::MigrationRatePerAnnum rpa;
					if (base > 0) {
						rpa = MigrationModel::calibrate_rate(base, base + migration, dt, absolute_rate);						
						if (!imr) {
							// model is not relative, so set the relative rate to 0
							rpa = MigrationModel::MigrationRatePerAnnum(0., rpa.absolute());
						}
					} else {
						rpa = MigrationModel::MigrationRatePerAnnum(0, 0);
						if (imr) {							
							LOG_WARN() << "MigrationCalibrator: because the base is zero, changing from relative to absolute rate";
							imr = false;
						}
					}
					MigrationModel model(MigrationModel::time_dependent_migration_rate(target_dt_begin, rpa));
					LOG_TRACE() << "MigrationCalibrator::add_migration_models: predicate " << pred->as_string() << " for rate per annum " << rpa << " and BASE=" << base;
					//if (target_dt_begin.year() == 2000) { // HACK
					//	LOG_DEBUG() << "Migration2000\t" << other_pred->as_string() << "\t" << ethn_rng << "\t" << rpa << "\t" << model_types[r];
					//}
					result.push_back(std::make_pair(std::move(pred), std::move(model)));
				}
			}

			std::vector<pred_model_pair> load_migration_models(const std::vector<std::pair<std::string, NumericalRange<int>>>& filenames_for_periods, const Ethnicity::IndexConversions& ic, CSV::Delimiter delim, const bool mid_year) {
				const size_t n = filenames_for_periods.size();
				std::vector<pred_model_pair> result;
				static const std::unordered_set<size_t> idx_col({ 0 });
				static const std::unordered_set<std::string> req_cols_data({ BASE_COL(), MIGRATION_COL() });
				static const std::unordered_set<std::string> req_cols_models({ MODEL_COL() });
				for (size_t i = 0; i < n; ++i) {
					CSVFileReader reader(filenames_for_periods[i].first, delim);										
					reader.select_columns(req_cols_data, idx_col);
					MigrationDataSet mds;
					mds.data = load_net_migration(reader, ic);
					if (!Inclusion::all_disjoint<false>(mds.data.index())) {
						throw DataException(boost::str(boost::format("MigrationCalibrator: ethnic group sets are not disjoint in file %s") % filenames_for_periods[i].first));
					}
					for (const std::string& req_col : req_cols_data) {
						if (!mds.data.has_col_label(req_col)) {
							throw DataException(boost::str(boost::format("MigrationCalibrator: column %s missing in file %s") % req_col % reader.file_name()));
						}
					}
					reader.reset_line_parser(delim);
					reader.select_columns(req_cols_models);
					if (reader.count_columns() < 1) {
						throw DataException(boost::str(boost::format("MigrationCalibrator: column %s missing in file %s") % MODEL_COL() % reader.file_name()));
					}
					mds.types = std::vector<std::string>(reader.begin(0), reader.end(0));
					mds.data_year_range = filenames_for_periods[i].second;
					add_migration_models(mds, mds.data_year_range, mid_year, result, nullptr);
				}
				return result;
			}

			static std::unique_ptr<MigrationGenerator> build_migration_generator(std::string&& name, const double scale_factor, unsigned int comigrated_child_age_limit, std::shared_ptr<const MigrantSelector> emigrant_selector, std::vector<pred_model_pair>&& models) {
				if (!models.empty()) {
					for (auto& p : models) {
						p.second.scale_base(scale_factor);
					}
				} else {
					LOG_WARN() << "MigrationCalibrator: no migration models";
				}
				return std::make_unique<MigrationGeneratorModel>(std::move(name), std::move(models), comigrated_child_age_limit, emigrant_selector);
			}

			std::unique_ptr<MigrationGenerator> build_migration_generator(std::string&& name, const std::vector < std::pair<std::string, NumericalRange<int>>>& filenames_for_periods, const Ethnicity::IndexConversions& ic, CSV::Delimiter delim, bool mid_year, const double scale_factor, unsigned int comigrated_child_age_limit, std::shared_ptr<const MigrantSelector> emigrant_selector) {
				check_not_null(emigrant_selector);
				check_that(scale_factor > 0.0, "MigrationCalibrator: Scale factor must be positive");
				LOG_DEBUG() << "MigrationCalibrator::build_migration_generator: scale_factor == " << scale_factor;
				return build_migration_generator(std::move(name), scale_factor, comigrated_child_age_limit, emigrant_selector, load_migration_models(filenames_for_periods, ic, delim, mid_year));
			}

			static void add_migration_models(const std::vector<pop_data_type>& actual, // historical population data
				// data from no-migration simulations:
				const std::vector<pop_data_type>& nomigr_start, // nomigr_start[i]: population state for i-th interval start
				const std::vector<pop_data_type>& nomigr_end, // nomigr_end[i]: population state for i-th interval end
				const std::vector<Date::year_type>& census_years, 
				const bool mid_year, 
				const unsigned int comigrated_child_age_limit, 
				const std::shared_ptr<const Predicate<Person>>& other_pred, 
				const Ethnicity::index_set_type& dominant_ethnic_groups, 
				const std::vector<std::unique_ptr<const Extender>>& extenders, 
				std::vector<pred_model_pair>& result)
			{
				const size_t n = actual.size(); // number of censuses
				check_that(n > 0);
				check_equals(n - 1, nomigr_start.size());
				check_equals(n - 1, nomigr_end.size());
				check_equals(n, census_years.size());
				const auto& age_groups = actual.front().index(); // vector of age groups
				check_equals<int, int, DataException>(RateCalibrator::MAX_AGE + 1, age_groups.back().end(), "MigrationCalibrator: last age group should extend to MAX_AGE");
				const auto& ethn_sets = actual.front().columns(); // vector of ethnic group sets
				// we consider ethnic group sets, not simply ethnic groups, because some censuses
				// reported data for lower-resolution ethnicity classification than the standard one 
				// assumed during the microsimulation; in which case we replace the lower-resolution
				// ethnic category (e.g. "WHITE") with an ethnic group set (e.g. ["WHITE_BRITISH", "IRISH", "OTHER_WHITE"])
				const size_t nagegrps = age_groups.size(); // number of age groups
				check_that<DataException>(nagegrps > 1, "MigrationCalibrator: algorithm requires at least 2 age groups");
				const size_t nethnsets = ethn_sets.size(); // number of ethnic group sets
				result.reserve(result.size() + nagegrps * (n - 1));
				MigrationDataSet migration_data_set; // recalculated for every age group and year, but allocated once
				migration_data_set.types.resize(nethnsets); // indicate which type 
				const static std::vector<std::string> data_col_labels({ BASE_COL(), MIGRATION_COL() });				
				data_type data_for_ethnic_groups(data_col_labels, ethn_sets); // recalculated for every age group, but allocated once
                Eigen::MatrixXd::ColXpr data_base_col = data_for_ethnic_groups.col_values(BASE_COL());
				Eigen::MatrixXd::ColXpr data_migration_col = data_for_ethnic_groups.col_values(MIGRATION_COL());
				typedef std::unordered_map<age_group_type, MigrationDataSet> data_map_type;
				TimeSeries<int, data_map_type> rescaled_data_maps;
				static const int year_step = 1;
				for (size_t sidx = 1; sidx < n; ++sidx) {
					for (auto year = census_years[sidx - 1]; year < census_years[sidx]; year = static_cast<Date::year_type>(year + year_step)) {
						rescaled_data_maps.push_back(std::make_pair(year, data_map_type()));
					}
				}
				for (size_t sidx = 1; sidx < n; ++sidx) {
					const auto& actual_base_df = actual[sidx - 1];
					const auto& actual_next_df = actual[sidx];
					const auto& nomigr_base_df = nomigr_start[sidx - 1];
					const auto& nomigr_next_df = nomigr_end[sidx - 1];
					check_equals(age_groups, actual_base_df.index());
					check_equals(ethn_sets, actual_base_df.columns());
					check_equals(age_groups, nomigr_next_df.index());
					check_equals(ethn_sets, nomigr_next_df.columns());
					const int age_delta = census_years[sidx] - census_years[sidx - 1];
					for (size_t agidx = 0; agidx < nagegrps - 1; ++agidx) {					
						const auto& age_group = age_groups[agidx];
						check_that<DataException>(age_group.begin() < age_group.end(), "MigrationCalibrator: age group cannot be empty");
						/*if (age_group.begin() < comigrated_child_age_limit) {
							check_that<DataException>(age_group.end() <= comigrated_child_age_limit, "MigrationCalibrator: age group must be either fully below or fully above the co-migration threshold");
							LOG_TRACE() << "MigrationCalibrator: skipping age group " << age_group << " because it is below to co-migration age threshold " << comigrated_child_age_limit;
							continue;
						}*/						
						age_group_type next_age_group = age_group + age_delta;
						LOG_TRACE() << "MigrationCalibrator: comparing age groups " << age_group << " and " << next_age_group;
						size_t next_age_group_index = nagegrps;
						age_group_type base_age_group = age_group;
						size_t next_agidx = agidx + 1;
						next_age_group_index = actual_next_df.row_idx_unsafe(next_age_group);
						if (next_age_group_index == actual_next_df.NOT_FOUND) { // get static constant via object
							// no such age group in next census data
							if (next_age_group.begin() >= age_groups.back().begin()) {
								for (size_t i = agidx + 1; i < nagegrps; ++i) {
									base_age_group = base_age_group + age_groups[i];
								}
								next_agidx = nagegrps;
								next_age_group = age_groups.back();
								next_age_group_index = nagegrps - 1;
								LOG_DEBUG() << "MigrationCalibrator: extended base age group to " << base_age_group << " and next age group to " << next_age_group;
							} else {
								// inconsistency
								LOG_ERROR() << "MigrationCalibrator: age group " << age_group << " has no counterpart " << next_age_group << " in next census data";
								throw DataException("MigrationCalibrator: inconsistent age group division");
							}
						}
						
						Eigen::VectorXd base_actual_row = actual_base_df.row_values_ix(agidx);
						Eigen::VectorXd base_nomigr_row = nomigr_base_df.row_values_ix(agidx);
						for (size_t i = agidx + 1; i < next_agidx; ++i) {
							base_actual_row += actual_base_df.row_values_ix(i);
							base_nomigr_row += nomigr_base_df.row_values_ix(i);
						}
						const auto next_actual_row = actual_next_df.row_values_ix(next_age_group_index);
						const auto next_nomigr_row = nomigr_next_df.row_values_ix(next_age_group_index);
						for (size_t eidx = 0; eidx < nethnsets; ++eidx) {
							const auto& ethn_set = ethn_sets[eidx];
							bool is_dominant = false;
							if (!Inclusion::is_disjoint_with(ethn_set, dominant_ethnic_groups)) {
								if (Inclusion::is_contained_by(ethn_set, dominant_ethnic_groups)) {
									is_dominant = true;
								} else {
									throw std::domain_error("MigrationCalibrator: incorrect ethnic grouping");
								}
							}
							const double base_actual = base_actual_row[eidx];
							data_base_col[eidx] = base_actual;
							if (base_nomigr_row[eidx] > 0) {								
								const double base_nomigr = base_nomigr_row[eidx];
								const double next_actual = next_actual_row[eidx];
								const double next_nomigr = next_nomigr_row[eidx];
								const double actual_delta = next_actual - base_actual;
								const double nomigr_delta = next_nomigr - base_nomigr;
								LOG_TRACE() << "MigrationCalibrator: base_age_group=" << base_age_group << ", sidx=" << sidx << ", eidx=" << eidx << ", actual_delta=" << actual_delta << ", nomigr_delta=" << nomigr_delta;
								const double migration = actual_delta - nomigr_delta;
								check_that<DataException>(base_actual >= 0);
								check_that<DataException>(-migration < base_actual);
								data_migration_col[eidx] = migration;
								if (is_dominant) {
									migration_data_set.types[eidx] = migration < 0 ? MODEL_RELATIVE : MODEL_ABSOLUTE;
								} else {
									migration_data_set.types[eidx] = MODEL_ABSOLUTE;
								}
							} else {
								LOG_WARN() << "MigrationCalibrator: base no-migration population is zero for group " << eidx << " as of " << census_years[sidx];
								migration_data_set.types[eidx] = MODEL_ABSOLUTE;
								data_migration_col[eidx] = 0;
							}
						}
						//static const int year_step = census_years[sidx] - census_years[sidx - 1];
						const double period_size_years = static_cast<double>(census_years[sidx] - census_years[sidx - 1]);
						for (int year = census_years[sidx - 1]; year < census_years[sidx]; year += year_step) {
							migration_data_set.data_year_range = NumericalRange<int>(year, year + year_step);
							age_group_type current_age_group = base_age_group + (year - census_years[sidx - 1]);
							if (current_age_group.end() <= comigrated_child_age_limit) {
								continue;
							}
							current_age_group = age_group_type(std::max(comigrated_child_age_limit, current_age_group.begin()), std::min(RateCalibrator::MAX_AGE + 1, current_age_group.end()));

							// Usually 1.0 unless clipped from below (co-migrated child age limit) or above (MAX_AGE)
							const double age_group_size_ratio = static_cast<double>(current_age_group.end() - current_age_group.begin()) / static_cast<double>(base_age_group.end() - base_age_group.begin());

							assert(age_group_size_ratio > 0);
							std::shared_ptr<const Predicate<Person>> pred = PredicateFactory::make_age(current_age_group.begin(), current_age_group.end() - 1, true);
							std::string subset_description("ALL");
							if (other_pred) {
								pred = PredicateFactory::make_and({ pred, other_pred });
								subset_description = other_pred->as_string();
							}
							migration_data_set.data = data_type(data_for_ethnic_groups * age_group_size_ratio);
                            Eigen::MatrixXd::ColXpr rescaled_data_migration_col = migration_data_set.data.col_values(MIGRATION_COL());
							const Eigen::MatrixXd::ColXpr rescaled_data_base_col = migration_data_set.data.col_values(BASE_COL());
							const double factor = static_cast<double>(year_step) / period_size_years;
							assert(factor > 0);
							for (size_t eidx = 0; eidx < nethnsets; ++eidx) {
								if (migration_data_set.types[eidx] == MODEL_ABSOLUTE) {
									rescaled_data_migration_col[eidx] *= factor;
								} else {
									assert(migration_data_set.types[eidx] == MODEL_RELATIVE);
									const double resc_base = rescaled_data_base_col[eidx];
									assert(resc_base > 0);
									double resc_migr = rescaled_data_migration_col[eidx];
									const double lograte = log1p(resc_migr / resc_base);
                                    resc_migr = resc_base * expm1(factor * lograte);
									if (resc_migr > 0) {
										LOG_WARN() << "MigrationCalibrator: using model RELATIVE for positive migration in age group " << current_age_group << ", year range " << migration_data_set.data_year_range << " and subset " << subset_description;
									}
                                    rescaled_data_migration_col[eidx] = resc_migr;
								}
							}
							LOG_DEBUG() << "MigrationCalibrator: generated a migration data frame for subset " << subset_description << ", age group " << current_age_group << ", year range " << migration_data_set.data_year_range << ":\n" << migration_data_set.data;
							LOG_DEBUG() << "and types:\n" << migration_data_set.types;
							(*rescaled_data_maps.value(year))[age_group] = migration_data_set;
							add_migration_models(migration_data_set, migration_data_set.data_year_range, mid_year, result, pred);
						}
						agidx = next_agidx - 1;
					}
				}
				for (const std::unique_ptr<const Extender>& extender : extenders) {
					extender->extend(rescaled_data_maps, other_pred, mid_year, result, comigrated_child_age_limit);
				}
			}

			std::unique_ptr<MigrationGenerator> build_migration_generator(std::string&& name,
				const std::vector<pop_data_type>& female_actual
				, const std::vector<pop_data_type>& male_actual
				, const std::vector<pop_data_type>& female_nomigr_start
				, const std::vector<pop_data_type>& male_nomigr_start
				, const std::vector<pop_data_type>& female_nomigr_end
				, const std::vector<pop_data_type>& male_nomigr_end
				, const std::vector<Date::year_type>& census_years, bool mid_year, double scale_factor, unsigned int comigrated_child_age_limit, std::shared_ptr<const MigrantSelector> emigrant_selector, const Ethnicity::index_set_type& dominant_ethnic_groups, const std::vector<std::unique_ptr<const Extender>>& extenders) {
				check_not_null(emigrant_selector);
				std::vector<pred_model_pair> result;
				add_migration_models(female_actual, female_nomigr_start, female_nomigr_end, census_years, mid_year, comigrated_child_age_limit, PredicateFactory::make_sex(Sex::FEMALE, true), dominant_ethnic_groups, extenders, result);
				add_migration_models(male_actual, male_nomigr_start, male_nomigr_end, census_years, mid_year, comigrated_child_age_limit, PredicateFactory::make_sex(Sex::MALE, true), dominant_ethnic_groups, extenders, result);
				return build_migration_generator(std::move(name), scale_factor, comigrated_child_age_limit, emigrant_selector, std::move(result));
			}

			Extender::~Extender() {}

			ExtenderAgeGroup::ExtenderAgeGroup(const std::vector<int>& src_years, const std::vector<double>& src_weights, int from_year, int to_year, const Ethnicity::index_set_type& ethn_groups, double multiplier_emigrants, double multiplier_immigrants)
				: ExtenderAgeGroup(src_years, src_weights, from_year, to_year, ethn_groups, TimeSeries<unsigned int, double>(0, multiplier_emigrants), TimeSeries<unsigned int, double>(0, multiplier_immigrants)) {}

			ExtenderAgeGroup::ExtenderAgeGroup(const std::vector<int>& src_years, const std::vector<double>& src_weights, int from_year, int to_year, const Ethnicity::index_set_type& ethn_groups, const TimeSeries<unsigned int, double>& multiplier_emigrants_by_age, const TimeSeries<unsigned int, double>& multiplier_immigrants_by_age)
				: src_years_(src_years), src_weights_(src_weights), from_year_(from_year), to_year_(to_year), ethn_groups_(ethn_groups), multiplier_emigrants_by_age_(multiplier_emigrants_by_age), multiplier_immigrants_by_age_(multiplier_immigrants_by_age) {
				check_equals(src_years.size(), src_weights.size());
				check_that(!src_years.empty(), "ExtenderAgeGroup: Source years are not empty empty");
				check_that(!multiplier_emigrants_by_age_.empty(), "ExtenderAgeGroup: No emigrant number multipliers");
				check_that(!multiplier_immigrants_by_age_.empty(), "ExtenderAgeGroup: No immigrant number multipliers");
				for (const auto& tv : multiplier_emigrants_by_age_) {
					check_that<std::out_of_range>(tv.second >= 0, "ExtenderAgeGroup: multiplier must be positive");
				}
				for (const auto& tv : multiplier_immigrants_by_age_) {
					check_that<std::out_of_range>(tv.second >= 0, "ExtenderAgeGroup: multiplier must be positive");
				}
			}

			void ExtenderAgeGroup::extend(const TimeSeries<int, std::unordered_map<age_group_type, MigrationDataSet>>& data_maps, const std::shared_ptr<const Predicate<Person>>& other_pred, bool mid_year, std::vector<pred_model_pair>& result, const unsigned int min_age) const {
				std::vector<const std::unordered_map<age_group_type, MigrationDataSet>*> src_datamaps;
				src_datamaps.reserve(src_years_.size());
				for (int src_year : src_years_) {
					const auto* data_map = data_maps.last_value(src_year);
					check_not_null(data_map, boost::str(boost::format("MigrationCalibrator::ExtenderAgeGroup: no data for year %d") % src_year).c_str());
					src_datamaps.push_back(data_map);
				}
				const NumericalRange<int> target_year_range(from_year_, std::min(to_year_ + 1, static_cast<int>(Date::MAX_YEAR)));
				const auto first_data_map_ptr = src_datamaps.front();
				const double first_src_weight = src_weights_.front();
				for (const auto& dm_ptr : src_datamaps) {
					check_equals(first_data_map_ptr->size(), dm_ptr->size(), "MigrationCalibrator::ExtenderAgeGroup: different number of age groups in data maps");
				}
				for (const auto& kv : *first_data_map_ptr) {
					if (min_age >= kv.first.end()) {
						continue;
					}
					std::shared_ptr<const Predicate<Person>> pred = PredicateFactory::make_age(std::max(kv.first.begin(), min_age), kv.first.end() - 1, true);
					if (other_pred) {
						pred = PredicateFactory::make_and({ pred, other_pred });
					}
					MigrationDataSet mds(filter_ethnic_groups(kv.second));					
					mds *= first_src_weight;
					for (size_t i = 1; i < src_years_.size(); ++i) {
						const auto other_kv = src_datamaps[i]->find(kv.first);
						check_that(other_kv != src_datamaps[i]->end());
						MigrationDataSet other_mds(filter_ethnic_groups(other_kv->second));
						other_mds.data_year_range = mds.data_year_range; // to make arithmetic operators happy
						other_mds *= src_weights_[i];
						mds += other_mds;
					}
					const age_group_type& age_grp = kv.first;
					const unsigned int median_age = (age_grp.begin() + age_grp.end()) / 2;
					auto col = mds.data.col_values(MIGRATION_COL());
					const double multiplier_immigrants = multiplier_immigrants_by_age_.padded_value(median_age);
					const double multiplier_emigrants = multiplier_emigrants_by_age_.padded_value(median_age);
					if (multiplier_emigrants != 1 || multiplier_immigrants != 1) {
						for (decltype(col.size()) i = 0; i < col.size(); ++i) {
							if (col[i] >= 0) { // immigration
								col[i] *= multiplier_immigrants;
							} else { // emigration
								col[i] *= multiplier_emigrants;
							}
						}
					}
					LOG_DEBUG() << "MigrationCalibrator: generated a migration data frame for subset " << pred->as_string() << ", age group " << kv.first << ", year range " << target_year_range << ":\n" << mds.data;
					LOG_DEBUG() << "and types:\n" << mds.types;
					add_migration_models(mds, target_year_range, mid_year, result, pred);
				}
			}

			MigrationDataSet ExtenderAgeGroup::filter_ethnic_groups(MigrationDataSet mds) const {
				if (!ethn_groups_.empty()) {
					const data_type::index_type groups = mds.data.index();
					for (const auto& egrp : groups) {
						const auto inclusion_relation = Inclusion::inclusion(egrp, ethn_groups_);
						if (inclusion_relation == InclusionRelation::DISJOINT) {
							// remove this group
							//LOG_DEBUG() << "MigrationCalibrator::ExtenderAgeGroup: removing ethnic group set " << egrp << " for age group " << kv.first;
							mds = mds.drop_ethnic_set(egrp);
						} else if (inclusion_relation != InclusionRelation::IS_CONTAINED_BY && inclusion_relation != InclusionRelation::EQUALS) {
							throw DataException(boost::str(boost::format("MigrationCalibrator::ExtenderAgeGroup: ethnic group set mismatch: set %s neither disjoint nor included in %s") % egrp % ethn_groups_));
						}
					}
				}
				return mds;
			}

			MigrationDataSet::MigrationDataSet() {}

			MigrationDataSet::MigrationDataSet(data_type&& n_data, std::vector<std::string>&& n_types, NumericalRange<int>&& n_data_year_range)
				: data(std::move(n_data)), types(std::move(n_types)), data_year_range(std::move(n_data_year_range)) {}

			MigrationDataSet MigrationDataSet::drop_ethnic_set(const Ethnicity::index_set_type& set) const {
				try {
					const size_t idx = data.row_idx(set);
					assert(idx < data.nbr_rows());
					assert(types.size() > 0);
					MigrationDataSet mds;
					mds.data_year_range = data_year_range;
					mds.data = data.drop_row_ix(idx);
					mds.types = types;
					mds.types.erase(mds.types.begin() + idx);
					return mds;
				} catch (std::out_of_range&) {
					return MigrationDataSet(*this);
				}
			}

			MigrationDataSet& MigrationDataSet::operator*=(double x) {
				data *= x;
				return *this;
			}

			MigrationDataSet MigrationDataSet::operator*(double x) const {
				return MigrationDataSet(data * x, std::vector<std::string>(types), NumericalRange<int>(data_year_range));
			}

			MigrationDataSet& MigrationDataSet::operator+=(const MigrationDataSet& other) {
				check_equals(types, other.types, "MigrationDataSet::operator+=: different types");
				check_equals(data_year_range, other.data_year_range, "MigrationDataSet::operator+=: different year ranges");
				data += other.data;
				return *this;
			}

			std::unique_ptr<MigrationGenerator> build_exodus_generator(std::string&& name, int year_from, int year_to, const Ethnicity::index_set_type& covered_groups, 
				std::shared_ptr<const MigrantSelector> migrant_selector, bool mid_year, double exodus_size, bool is_relative, double scale_factor, 
				unsigned int comigrated_child_age_limit, const std::shared_ptr<const Predicate<Person>> exodus_predicate) {
				check_that(year_from < year_to);
				check_that(!covered_groups.empty());
				check_not_null(migrant_selector);
				check_that(exodus_size >= 0);
				const Date from = make_migration_date(year_from, mid_year);
				const Date to = make_migration_date(year_to, mid_year);
				const double dt = MigrationModel::calc_dt(from, to);
				double absolute_rate = 0;
				double relative_rate = 0;
				if (is_relative) {
					// (1 - exodus_size) = exp(relative_rate * dt)
					// log(1 - exodus_size) = relative_rate * dt
					relative_rate = log1p(-exodus_size) / dt;
				} else {
					absolute_rate = - exodus_size * scale_factor / dt;
				}				
				assert(relative_rate <= 0);
				assert(absolute_rate <= 0);
				const MigrationModel::MigrationRatePerAnnum rpa(relative_rate, absolute_rate);
				LOG_DEBUG() << "MigrationCalibrator::build_exodus_generator: converted " << (is_relative ? "relative" : "absolute") << " exodus size into RPA=" << rpa;
				MigrationModel::time_dependent_migration_rate rpa_series(from, rpa);
				rpa_series.push_back(to, MigrationModel::MigrationRatePerAnnum(0., 0.));
				MigrationModel model(std::move(rpa_series));
				std::unique_ptr<const Predicate<Person>> predicate = PredicateFactory::make_and<Person>({
					PredicateFactory::make_asof<Person>(from, to),
					PredicateFactory::make_ethnicity(covered_groups, true)				
				});
				if (exodus_predicate) {
					predicate = PredicateFactory::make_and<Person>({
						std::shared_ptr<const Predicate<Person>>(std::move(predicate)),
						exodus_predicate
					});
				}
				std::vector<MigrationGeneratorModel::pred_model_pair> pred_model;
				pred_model.push_back(std::make_pair(std::move(predicate), std::move(model)));
				return std::make_unique<MigrationGeneratorModel>(std::move(name), std::move(pred_model), comigrated_child_age_limit, migrant_selector);
			}

			std::unique_ptr<MigrationGenerator> build_return_generator(std::string&& name, int return_year_from, int return_year_to, bool mid_year, unsigned int returning_children_age_limit, std::unique_ptr<EmigrantSelector>&& emigrant_selector, double returning_fraction) {
				const Date from = make_migration_date(return_year_from, mid_year);
				const Date to = make_migration_date(return_year_to, mid_year);
				return std::make_unique<MigrationGeneratorReturn>(std::move(name), from, to, returning_fraction, returning_children_age_limit, std::move(emigrant_selector));
			}
		}
	}
}

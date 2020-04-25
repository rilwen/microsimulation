/*
Microsimulation runner for Brexit scenarios simulation in England and Wales.

(C) Averisera Ltd 2016-2020
*/
#include "microsim-core/anchored_hazard_curve.hpp"
#include "microsim-uk/ethnicity/ethnicity_ons_full.hpp"
#include "microsim-core/conception.hpp"
#include "microsim-core/schedule_definition.hpp"
#include "microsim-core/schedule.hpp"
#include "microsim-calibrator/migration_calibrator.hpp"
#include "microsim-calibrator/mortality_calibrator.hpp"
#include "microsim-calibrator/population_calibrator.hpp"
#include "microsim-calibrator/procreation_calibrator.hpp"
#include "microsim-calibrator/stitched_markov_model_calibrator.hpp"
#include "microsim-simulator/contexts.hpp"
#include "microsim-simulator/common_features.hpp"
#include "microsim-simulator/hazard_rate_multiplier_provider/hazard_rate_multiplier_provider_bypred_timedependent.hpp"
#include "microsim-simulator/immutable_context.hpp"
#include "microsim-simulator/initialiser/initialiser_generations.hpp"
#include "microsim-simulator/migration/emigrant_selector.hpp"
#include "microsim-simulator/migration/migrant_selector_random.hpp"
#include "microsim-simulator/migration/migrant_selector_latest_immigrants_first.hpp"
#include "microsim-simulator/migration/migration_generator_dummy.hpp"
#include "microsim-simulator/observer/observer_demographics_main.hpp"
#include "microsim-simulator/observer/observer_demographics_immigrants.hpp"
#include "microsim-simulator/observer/observer_demographics_emigrants.hpp"
#include "microsim-simulator/observer/observer_stats.hpp"
#include "microsim-simulator/observer/observer_result_saver_simple.hpp"
#include "microsim-simulator/operator_factory.hpp"
#include "microsim-simulator/operator/operator_discrete_independent.hpp"
#include "microsim-simulator/operator/operator_function.hpp"
#include "microsim-simulator/operator/mortality.hpp"
#include "microsim-simulator/person_data.hpp"
#include "microsim-simulator/population.hpp"
#include "microsim-simulator/population_data.hpp"
#include "microsim-simulator/population_loader.hpp"
#include "microsim-simulator/procreation.hpp"
#include "microsim-simulator/predicate_factory.hpp"
#include "microsim-simulator/simulator.hpp"
#include "microsim-simulator/simulator_builder.hpp"
#include "microsim-uk/ethnicity/ethnicity_classifications_england_wales.hpp"
#include "microsim-uk/state_pension_age.hpp"
#include "microsim-uk/state_pension_age_2007.hpp"
#include "core/csv_file_reader.hpp"
#include "core/distribution_shifted_lognormal.hpp"
#include "core/math_utils.hpp"
#include "core/period.hpp"
#include "core/preconditions.hpp"
#include "core/user_arguments.hpp"
#include <string>

using namespace averisera;
using namespace averisera::microsim;

const CSV::Delimiter DELIM = CSV::Delimiter::TAB; /**< Type of delimiter used in input files. */
const bool MIGRATION_MID_YEAR = true; /**< Whether migration happens on 1 July (true) or 1 January (false) each year. */
const bool USE_SINGLE_SIMULATION_WITHOUT_MIGRATION = false; /**< Whether migration is estimated by performing a single simulation without it between the 1st and last census year (true), or by performing such migration-free simulations between consecutive census years. */
const std::string BMI_CATEGORY_VARIABLE_NAME("BMICat"); /**< History variable name for BMI category. */
const std::string BMI_PERCENTILE_VARIABLE_NAME("BMIPerc"); /**< History variable name for BMI percentile. */
const std::string BMI_VARIABLE_NAME("BMI"); /**< History variable name for BMI value. */
const unsigned int POST_PREGNANCY_ZERO_FERTILITY_MONTHS = 3; /**< How many months after pregnancy are assumed to be zero fertility. */
const double POST_PREGNANCY_ZERO_FERTILITY_YEAR_FRACTION = static_cast<double>(POST_PREGNANCY_ZERO_FERTILITY_MONTHS) / 12.0 - 0.0001; // Subtract small fraction to help calibration.

typedef uint8_t bmi_cat_type; /**< Type used to represent BMI categories. */

/** Loads mortality rates from data file.
*/
static std::vector<std::unique_ptr<AnchoredHazardCurve>> load_mortality_data(const std::string& mortality_rates_file, const Schedule& schedule, const RateCalibrator::age_type max_age) {
	CSVFileReader rates(mortality_rates_file, DELIM, CSV::QuoteCharacter::DOUBLE_QUOTE);
	Date::year_type max_year_of_birth = schedule.end_date().year();
	Date::year_type min_year_of_birth = MathUtils::safe_cast<Date::year_type>(schedule.begin()->begin.year() - max_age);
	return MortalityCalibrator::calc_mortality_curves(rates, min_year_of_birth, max_year_of_birth);
}

/** Builds operators applying mortality to people.
*/
static std::vector<std::unique_ptr<Operator<Person>>> build_mortality_operators(const std::string& mortality_rates_file, const Schedule& schedule, const RateCalibrator::age_type max_age, const std::shared_ptr<const Predicate<Person>>& predicate) {
	auto vec = Mortality::build_operators(std::move(load_mortality_data(mortality_rates_file, schedule, max_age)), nullptr, predicate);
	std::vector<std::unique_ptr<Operator<Person>>> result(vec.size());
	std::transform(vec.begin(), vec.end(), result.begin(), [](std::unique_ptr<Mortality>& m) { return std::move(m); });
	return result;
}

/** Returns multipliers correcting fertility rates for ethnic groups.
*/
static std::unique_ptr<const HazardRateMultiplierProvider<Person>> get_total_fertility_rates_multipliers(const std::string& filename, const std::string& ethnicity_classification) {
	CSVFileReader reader(filename, DELIM);	
	Ethnicity::IndexConversions ic(EthnicityClassficationsEnglandWales::get_conversions(ethnicity_classification.c_str()));
	typedef DataFrame<Range<PersonAttributes::ethnicity_t>, NumericalRange<int>> df_type;
	const df_type tfr(ProcreationCalibrator::load_total_fertility_rates_for_ethnic_groups(reader, ic));
	std::vector<HazardRateMultiplierProviderByPredTimeDependent<Person>::pred_hrm_ser_pair> pairs;
	pairs.reserve(tfr.nbr_cols());
	const auto col_label_all = ic.index_range_all();
	if (!tfr.has_col_label(col_label_all)) {
		throw DataException(boost::str(boost::format("No data for total population in file %s") % filename));
	}
	const auto column_all = tfr.col_values(col_label_all);
	for (df_type::size_type c = 0; c < tfr.nbr_cols(); ++c) {
		const auto group_range = tfr.columns()[c];
		std::unique_ptr<const Predicate<Person>> pred(PredicateFactory::make_ethnicity(static_cast<PersonAttributes::ethnicity_t>(group_range.begin()), static_cast<PersonAttributes::ethnicity_t>(group_range.end()), true));
		HazardRateMultiplierProviderByPredTimeDependent<Person>::hrm_series series;
		series.reserve(tfr.nbr_rows());
		for (df_type::size_type r = 0; r < tfr.nbr_rows(); ++r) {
			const auto year_range = tfr.index()[r];
			const double multiplier = tfr.ix(r, c) / column_all[r];
			series.push_back(Date(static_cast<Date::year_type>(year_range.begin()), 1, 1), HazardRateMultiplier(multiplier)); // multiplier affects the whole life
		}
		pairs.push_back(std::make_pair(std::move(pred), std::move(series)));
	}
	// TODO: ideally, remaining ethnic groups should have their fertility rates adjusted to match other data
	// but the error we're making by not doing so is small.
	for (const auto& pair : pairs) {
		LOG_DEBUG() << "HRM pair: " << pair.first->as_string() << ", " << pair.second;
	}
	return std::make_unique<HazardRateMultiplierProviderByPredTimeDependent<Person>>(std::move(pairs));
}

/** Builds operators handling conceiving babies.

@param multiplicity_distros Vector of TimeSeries (age, multiplicity distribution) indexed by years
*/
static std::vector<std::unique_ptr<Operator<Person>>> build_conception_operators(const std::string& birth_rates_file, const double birth_rate_basis, const Conception::mdistr_multi_series_type& multiplicity_distros, const std::unique_ptr<const HazardRateMultiplierProvider<Person>>& hrm_provider) {	
	CSVFileReader reader(birth_rates_file, DELIM);
	const auto birth_rates = ProcreationCalibrator::load_cohort_birth_rates(reader, birth_rate_basis, true);
	// hazard rates for cohorts
	const auto cohort_conception_hazard_rates = ProcreationCalibrator::calculate_conception_hazard_rates(birth_rates, multiplicity_distros, POST_PREGNANCY_ZERO_FERTILITY_YEAR_FRACTION);
	auto conception_hazard_curves = ProcreationCalibrator::calculate_conception_hazard_curves(cohort_conception_hazard_rates);
	const size_t ncohorts = birth_rates.index().size();
	std::vector<std::unique_ptr<Operator<Person>>> operators(ncohorts);
	for (size_t cidx = 0; cidx < ncohorts; ++cidx) {
		Conception::mdistr_multi_series_type mdcopy(multiplicity_distros);
		Conception conception(std::shared_ptr<const AnchoredHazardCurve>(std::move(conception_hazard_curves[cidx])), std::move(mdcopy));
		std::vector<std::shared_ptr<const RelativeRisk<Person>>> rrs;
		std::shared_ptr<const Predicate<Person>> predicate;
		assert(ncohorts > 0);
		if (ncohorts > 1) {
			if (cidx == 0) {
				predicate = PredicateFactory::make_year_of_birth(Date::MIN_YEAR, cohort_conception_hazard_rates.index()[cidx + 1] - 1);
			} else if (cidx == ncohorts - 1) {
				predicate = PredicateFactory::make_year_of_birth(cohort_conception_hazard_rates.index()[cidx], Date::MAX_YEAR);
			} else {
				predicate = PredicateFactory::make_year_of_birth(cohort_conception_hazard_rates.index()[cidx], cohort_conception_hazard_rates.index()[cidx + 1] - 1);
			}
		}
		assert(predicate != nullptr);
		// try with default schedule for now
		auto operator_ptr = OperatorFactory::make_conception(conception, rrs, predicate, nullptr, hrm_provider, ProcreationCalibrator::MIN_CHILDBEARING_AGE, ProcreationCalibrator::MAX_CHILDBEARING_AGE, Period::months(POST_PREGNANCY_ZERO_FERTILITY_MONTHS));
		operators[cidx] = std::move(operator_ptr);
	}
	return operators;
}

/** Builds operators handling creating fetuses after conception (gender ratios). */
static std::vector<std::unique_ptr<Operator<Person>>> build_fetus_generator_operators(const std::string& birth_gender_ratios_file) {
	CSVFileReader reader(birth_gender_ratios_file, DELIM);
	const auto gender_rates = ProcreationCalibrator::load_gender_rates(reader, 0);
	TimeSeries<Date, double> probs_female;
	probs_female.reserve(gender_rates.nbr_rows());
	const auto female_ratios = gender_rates.col_values(Sex::FEMALE);
	for (size_t r = 0; r < gender_rates.nbr_rows(); ++r) {
		const int year = gender_rates.index()[r];
		const double p = female_ratios[r];
		if (p >= 0 && p <= 1) {
			probs_female.push_back(Date(MathUtils::safe_cast<Date::year_type>(year - 1), 9, 1), p);
		} else {
			throw DataException("Gender ratios out of bounds");
		}
	}
	std::vector<std::unique_ptr<Operator<Person>>> operators(1);
	operators[0] = OperatorFactory::make_fetus_generator_simple(std::move(probs_female), ProcreationCalibrator::MIN_CHILDBEARING_AGE, ProcreationCalibrator::MAX_CHILDBEARING_AGE);
	return operators;
}

/** Generates names of files with census data. */
static std::vector<std::string> get_census_filenames(const std::vector<Date::year_type>& census_years_start, const unsigned int census_spacing_years, const std::string& resource_dir, const std::string& pattern, const bool single_sim) {
	std::vector<std::string> census_filenames;
	for (auto yr_start : census_years_start) {
		if (single_sim) {
			census_filenames.push_back(resource_dir + boost::str(boost::format(pattern) % yr_start));
		} else {
			const auto yr_end = yr_start + census_spacing_years;
			census_filenames.push_back(resource_dir + boost::str(boost::format(pattern) % yr_start % yr_end));
		}
	}
	return census_filenames;
}

/** Rescales values in a range by a constant factor. */
template <class It> static void rescale_range(It begin, const It end, double factor) {
	for (; begin != end; ++begin) {
		(*begin) *= factor;
	}
}

typedef std::vector<PopulationCalibrator::pop_data_type> pop_data_vector;

/** Calculates total population size */
static double calc_total_population(const PopulationCalibrator::pop_data_type& female, const PopulationCalibrator::pop_data_type& male) {
	return female.values().sum() + male.values().sum();
}

/** Loads and rescales (to match historical starting population) simulated no-migration census numbers coming from a single simulation. */
static void get_single_simulation_no_migration_numbers(const std::vector<Date::year_type>& census_years_start,
	const std::string& resource_dir,
	const Ethnicity::IndexConversions& ic,
	const double total_historical_population_start,
	pop_data_vector& nomigr_female_census_numbers_start,
	pop_data_vector& nomigr_male_census_numbers_start,
	pop_data_vector& nomigr_female_census_numbers_end,
	pop_data_vector& nomigr_male_census_numbers_end
	) {
	const std::vector<std::string> nomigr_female_census_filenames(get_census_filenames(census_years_start, 0, resource_dir, "nomigr_census_%d_female.csv", true));
	const std::vector<std::string> nomigr_male_census_filenames(get_census_filenames(census_years_start, 0, resource_dir, "nomigr_census_%d_male.csv", true));
	pop_data_vector nomigr_female_census_numbers(PopulationCalibrator::load_population_numbers(nomigr_female_census_filenames, DELIM, ic));
	pop_data_vector nomigr_male_census_numbers(PopulationCalibrator::load_population_numbers(nomigr_male_census_filenames, DELIM, ic));
	if (nomigr_female_census_numbers[0].index() != nomigr_male_census_numbers[0].index()) {
		throw DataException("Female and male no-migration census numbers have different age groups");
	}
	const double total_nomigr_population_start = calc_total_population(nomigr_female_census_numbers[0], nomigr_male_census_numbers[0]);
	LOG_INFO() << "Total no-migration census population at start: " << total_nomigr_population_start;
	check_that<DataException>(total_nomigr_population_start > 0.0, "Total no-migration population must be positive");
	const double census_factor = total_historical_population_start / total_nomigr_population_start;
	rescale_range(nomigr_female_census_numbers.begin(), nomigr_female_census_numbers.end(), census_factor);
	rescale_range(nomigr_male_census_numbers.begin(), nomigr_male_census_numbers.end(), census_factor);

	nomigr_female_census_numbers_start = pop_data_vector(nomigr_female_census_numbers.begin(), nomigr_female_census_numbers.end() - 1);
	nomigr_male_census_numbers_start = pop_data_vector(nomigr_male_census_numbers.begin(), nomigr_male_census_numbers.end() - 1);
	nomigr_female_census_numbers_end = pop_data_vector(nomigr_female_census_numbers.begin() + 1, nomigr_female_census_numbers.end());
	nomigr_male_census_numbers_end = pop_data_vector(nomigr_male_census_numbers.begin() + 1, nomigr_male_census_numbers.end());
}

/** Loads and rescales (to match historical census populations) simulated no-migration census numbers coming from a 10-year simulations spanning the intervals between historical censuses. */
static void get_multiple_simulations_no_migration_numbers(const std::vector<Date::year_type>& census_years,
	const unsigned int census_spacing_years,
	const std::string& resource_dir,
	const Ethnicity::IndexConversions& ic,
	const std::vector<double>& total_historical_population_sizes,
	pop_data_vector& nomigr_female_census_numbers_start,
	pop_data_vector& nomigr_male_census_numbers_start,
	pop_data_vector& nomigr_female_census_numbers_end,
	pop_data_vector& nomigr_male_census_numbers_end) {
	const std::vector<Date::year_type> census_years_start(census_years.begin(), census_years.end() - 1);
	const std::vector<std::string> nomigr_female_census_filenames_start(get_census_filenames(census_years_start, census_spacing_years, resource_dir, "nomigr_census_%d_%d_female_start.csv", false));
	const std::vector<std::string> nomigr_male_census_filenames_start(get_census_filenames(census_years_start, census_spacing_years, resource_dir, "nomigr_census_%d_%d_male_start.csv", false));
	//const std::vector<Date::year_type> census_years_end(census_years.begin() + 1, census_years.end());
	const std::vector<std::string> nomigr_female_census_filenames_end(get_census_filenames(census_years_start, census_spacing_years, resource_dir, "nomigr_census_%d_%d_female_end.csv", false));
	const std::vector<std::string> nomigr_male_census_filenames_end(get_census_filenames(census_years_start, census_spacing_years, resource_dir, "nomigr_census_%d_%d_male_end.csv", false));
	nomigr_female_census_numbers_start = PopulationCalibrator::load_population_numbers(nomigr_female_census_filenames_start, DELIM, ic);
	nomigr_male_census_numbers_start = PopulationCalibrator::load_population_numbers(nomigr_male_census_filenames_start, DELIM, ic);
	nomigr_female_census_numbers_end = PopulationCalibrator::load_population_numbers(nomigr_female_census_filenames_end, DELIM, ic);
	nomigr_male_census_numbers_end = PopulationCalibrator::load_population_numbers(nomigr_male_census_filenames_end, DELIM, ic);
	for (size_t i = 0; i < census_years_start.size(); ++i) {
		if (nomigr_female_census_numbers_start[i].index() != nomigr_male_census_numbers_start[i].index()) {
			throw DataException("Female and male no-migration census numbers have different age groups");
		}
		const double ref = total_historical_population_sizes[i];
		const double nomigr = calc_total_population(nomigr_female_census_numbers_start[i], nomigr_male_census_numbers_start[i]);
		LOG_INFO() << "Total reference census population at start (" << census_years_start[i] << "): " << ref;
		LOG_INFO() << "Total no-migration census population at start (" << census_years_start[i] << "): " << nomigr;
		check_that<DataException>(nomigr > 0.0, "Total no-migration population must be positive");
		const double scale_factor = ref / nomigr;
		nomigr_female_census_numbers_start[i] *= scale_factor;
		nomigr_male_census_numbers_start[i] *= scale_factor;
		nomigr_female_census_numbers_end[i] *= scale_factor;
		nomigr_male_census_numbers_end[i] *= scale_factor;
	}
}

/** Generates sets of ethnic groups for BMI modelling */
std::map<std::string, Ethnicity::index_set_type> get_bmi_ethnic_sets(const Ethnicity::IndexConversions& ic) {
	const Ethnicity::index_set_type white({ ic.index("WHITE_BRITISH"), ic.index("IRISH"), ic.index("GYPSY_OR_TRAVELLER"), ic.index("OTHER_WHITE") });
	const Ethnicity::index_set_type asian({ ic.index("INDIAN"), ic.index("PAKISTANI"), ic.index("BANGLADESHI"), ic.index("CHINESE"), ic.index("OTHER_ASIAN") });
	Ethnicity::index_set_type other;
	for (Ethnicity::IndexConversions::index_type i = 0; i < ic.size(); ++i) {
		if ((white.find(i) == white.end()) && (asian.find(i) == asian.end())) {
			other.insert(i);
		}
	}
	std::map<std::string, Ethnicity::index_set_type> map;
	map["WHITE"] = white;
	map["ASIAN"] = asian;
	map["OTHER"] = other;
	return map;
}


/** Builds operators handling BMI dynamics. */
static std::vector<std::unique_ptr<Operator<Person>>> build_bmi_operators(const std::string& csm_calibration_file, 
	const bmi_cat_type dim, 
	const RateCalibrator::age_type min_age,
	const RateCalibrator::age_type max_age,
	const Date::year_type min_year,
	const Date::year_type max_year,
	const Date::month_type month, 
	const Date::day_type day,
	const std::map<std::string, Ethnicity::index_set_type>& ethnic_groupings,
	const std::string& category_variable_name,
	const std::string& percentile_variable_name,
	const CSV::Delimiter csv_delimiter,
	const bool do_initialisation,
	const bool generate_continuous_values,
	const std::vector<double>& bmi_thresholds,
	const std::string& continuous_variable_name,
	const double max_bmi,
	const bool store_percentiles_as_floats) {
	CSVFileReader reader(csm_calibration_file, csv_delimiter, CSV::QuoteCharacter::DOUBLE_QUOTE);
	std::vector<StitchedMarkovModelWithSchedule<bmi_cat_type>> models;
	std::vector<Cohort::yob_ethn_sex_cohort_type> cohorts;
	StitchedMarkovModelCalibrator::calibrate_annual_models<bmi_cat_type>(min_age, static_cast<Date::year_type>(min_year - max_age), min_year, max_year, dim, month, day, reader, models, cohorts);
	static const unsigned int period_years = 1;
	const size_t n = models.size();
	LOG_INFO() << "Calibrated " << n << " BMI models for " << cohorts.size() << " cohorts";
	const Date end_date = Date(max_year, month, day);
	std::vector<Date> start_dates(n);
	std::transform(models.begin(), models.end(), start_dates.begin(), [](const StitchedMarkovModelWithSchedule<bmi_cat_type>& model) {return model.start_date(); });
	std::vector<std::unique_ptr<Operator<Person>>> operators(OperatorDiscreteIndependent<bmi_cat_type>::build_for_cohorts(ethnic_groupings, category_variable_name, do_initialisation, percentile_variable_name, models, cohorts, start_dates, period_years, end_date, store_percentiles_as_floats));
	LOG_INFO() << "Built " << operators.size() << " BMI operators from " << models.size() << " BMI models";
	if (generate_continuous_values) {
		check_equals<size_t, unsigned int, DataException>(bmi_thresholds.size(), dim, "Number of BMI thresholds must match the BMI model dimension");	
		std::vector<std::vector<OperatorFunction<Person>::function_type>> functions(n);
		for (size_t i = 0; i < n; ++i) {
			//const auto& cohort = cohorts[i];
			std::vector<OperatorFunction<Person>::function_type> fvec;
			const auto& model = models[i];
			static const Period delta = model.period();
			assert(delta.size == period_years);
			assert(delta.type == PeriodType::YEARS);
			const Date start_date = start_dates[i];
			if (start_date.year() <= max_year) {
				const size_t len = static_cast<size_t>(max_year - start_date.year() + 1) / period_years;
				fvec.resize(len);
				Date date = start_dates[i];				
				for (size_t t = 0; t < len; ++t) {
					const auto distr_discr = model.calc_state_distribution(date);
					assert(bmi_thresholds.size() == static_cast<size_t>(distr_discr.size()));
					std::vector<double> probs(dim); // copy into a vector
					for (bmi_cat_type k = 0; k < dim; ++k) {
						probs[k] = distr_discr[k];
					}
					const std::shared_ptr<const Distribution> distr_cont(std::make_shared<DistributionShiftedLognormal>(DistributionShiftedLognormal::estimate_given_shift(bmi_thresholds, probs)));
					fvec[t] = Distribution::icdf_as_function(distr_cont, max_bmi);
					date = date + delta;
 				}
			} // else leave functions[i] empty
			functions[i] = std::move(fvec);
		}
		const auto bmi_value_history_factory = store_percentiles_as_floats ? HistoryFactory::SPARSE<double>() : HistoryFactory::SPARSE<float>(); // store BMI percentiles and value as the same type
		std::vector<std::unique_ptr<Operator<Person>>> continuous_value_operators(build_operator_function_for_cohorts(ethnic_groupings, percentile_variable_name, continuous_variable_name, functions, period_years, cohorts, start_dates, bmi_value_history_factory));
		LOG_INFO() << "Built " << continuous_value_operators.size() << " continuous BMI value operators";
		operators.insert(operators.end(), std::make_move_iterator(continuous_value_operators.begin()), std::make_move_iterator(continuous_value_operators.end())); // move values from continuous_value_operators to the end of operators
	}
	return operators;
}

/** Main function. */
void do_main(const UserArguments& ua) {
    // Read user arguments
    //const std::string variables_filename(ua.get<std::string>("VARIABLES_FILE"));
    //const std::string persons_filename(ua.get<std::string>("PERSONS_FILE"));
	const Date start_date = ua.get<Date>("START_DATE");
	const unsigned int census_year_spacing = ua.get("CENSUS_INTERVAL_YEARS", 10);
	const Date end_date = ua.get<Date>("END_DATE");
	const Date::year_type start_year = start_date.year();
	if (start_year != 1991 && start_year != 2001) {
		throw DataException("Simulation has to start in 1991 or 2001");
	}
	ScheduleDefinition schedule_definition(start_date,
		end_date,
		ua.get<Period>("FREQUENCY"),
		ua.get<std::shared_ptr<const Daycount>>("DAYCOUNT"));
	const Initialiser::pop_size_t init_pop_size = MathUtils::safe_cast<Initialiser::pop_size_t>(ua.get<double>("INIT_POPULATION_SIZE"));
	const std::string observations_filename(ua.get<std::string>("OBSERVATIONS_FILE"));
	std::vector<std::string> variables_for_stats;
	ua.get("OBSERVED_STATS_VARIABLES", variables_for_stats, false);
	const bool calc_medians = ua.get("CALC_MEDIANS", false);
	std::string resource_dir = ua.get("RESOURCE_DIR", std::string("resources/"));
	if (resource_dir.empty()) {
		resource_dir = ".";
	}
	if (resource_dir.back() != '/') {
		resource_dir += "/";
	}
	const RateCalibrator::age_type max_age = ua.get<RateCalibrator::age_type>("MAX_AGE", 100);
	const double multiple_birth_basis = ua.get<double>("MULTIPLE_BIRTH_BASIS");	
	const double birth_rate_basis = ua.get<double>("BIRTH_RATE_BASIS");
	const std::string cohort_fertility_rates_filename = ua.get<std::string>("COHORT_FERTILITY_FILE", "cohort_fertility_rates.csv");
	
	const std::string male_mortality_filename(ua.get<std::string>("MALE_MORTALITY_FILE", "mortality_male.csv"));
	const std::string female_mortality_filename(ua.get<std::string>("FEMALE_MORTALITY_FILE", "mortality_female.csv"));
	
	const std::string ethnicity_classification(ua.get<std::string>("ETHNICITY_CLASSIFICATION"));
	std::vector<std::string> eu_ethnic_group_names;
	ua.get("EU_ETHNIC_GROUPS", eu_ethnic_group_names, false);
	if (eu_ethnic_group_names.empty()) {
		eu_ethnic_group_names.push_back("OTHER_WHITE");
	}

	const bool do_migration = ua.get("DO_MIGRATION", true); // model migration
	const bool accurate_migration = ua.get("ACCURATE_MIGRATION", true); // ... accurately
	const unsigned int comigrated_child_age_limit = ua.get("COMIGRATED_CHILD_AGE_LIMIT", 10u);

	const bool do_bmi = ua.get("DO_BMI", false); // model BMI
	const unsigned int bmi_min_age = ua.get("BMI_MIN_AGE", 15u);
	std::vector<std::string> bmi_categories;
	const bool do_continuous_bmi = ua.get("DO_CONTINUOUS_BMI", false);
	const double max_bmi = ua.get("MAX_BMI", 80.0);
	std::vector<double> bmi_thresholds;
	if (do_bmi) {
		ua.get("BMI_CATS", bmi_categories, true);
		if (do_continuous_bmi) {
			ua.get("BMI_THRESHOLDS", bmi_thresholds, true); // minimum BMI, 1st threshold, 2nd threshold...
			check_equals<size_t, size_t, DataException>(bmi_categories.size(), bmi_thresholds.size(), "Number of BMI categories and thresholds must be the same");
		}
	}
	const bool store_bmi_percentiles_as_floats = init_pop_size < 10000000;
	const auto bmi_dim = static_cast<bmi_cat_type>(bmi_categories.size());

	const bool do_spa = ua.get<bool>("DO_SPA"); // model State Pension Age
	const bool use_spa2007 = ua.get<bool>("USE_SPA2007", false); // use the State Pension Age reform from 2007

	const bool do_brexit = ua.get<bool>("BREXIT"); // simulate Brexit scenario
	const bool only_calibration = ua.get<bool>("ONLY_CALIBRATION", false); // run only calibration and exit
	const int brexit_year = ua.get("BREXIT_YEAR", 2019); // year when Brexit happens (assumed to be mid-year)
	const double post_brexit_eu_migration_multiplier = ua.get("POSTBREXIT_EU_MIGR_MULTI", 1.0);
	const double post_brexit_other_migration_multiplier = ua.get("POSTBREXIT_OTHER_MIGR_MULTI", 1.0);
	const double post_brexit_dominant_emigration_multiplier = ua.get("POSTBREXIT_DOMINANT_EMIGR_MULTI", 1.0);
	const double post_brexit_dominant_return_multiplier = ua.get("POSTBREXIT_DOMINANT_RETURN_MULTI", 0.0);
	const double post_brexit_exodus_size = ua.get("POSTBREXIT_EXODUS_SIZE", 0.0);
	const bool is_post_brexit_exodus_size_relative = ua.get("IS_POSTBREXIT_EXODUS_SIZE_RELATIVE", true);
	const int post_brexit_exodus_length_years = ua.get("POSTBREXIT_EXODUS_LENGTH_YEARS", 1);
	const int post_brexit_return_length_years = ua.get("POSTBREXIT_RETURN_LENGTH_YEARS", 1);
	check_that<DataException>(post_brexit_exodus_size >= 0, "Exodus size cannot be negative");
	check_that<DataException>(post_brexit_dominant_return_multiplier >= 0, "Post-Brexit return multiplier cannot be negative");
	check_that<DataException>(post_brexit_eu_migration_multiplier >= 0, "Post-Brexit EU migration multiplier cannot be negative");
	check_that<DataException>(post_brexit_dominant_emigration_multiplier >= 0, "Post-Brexit dominant group emigration multiplier cannot be negative");
	check_that<DataException>(post_brexit_exodus_length_years > 0, "Exodus length in years must be positive");
	check_that<DataException>(post_brexit_return_length_years > 0, "Return length in years must be positive");

	const int future_eu_enlargement_year = ua.get("FUTURE_EU_ENLARGEMENT_YEAR", static_cast<int>(Date::MAX_YEAR));
	const bool do_future_eu_enlargement = ua.get("FUTURE_EU_ENLARGEMENT", false);
	const double post_future_enlargement_eu_migration_multiplier = ua.get("POST_FUTURE_EU_ENLARGEMENT_MIGR_MULTI", 1.0);
	const int post_future_enlargement_eu_migration_length_years = ua.get("POST_FUTURE_EU_ENLARGEMENT_MIGR_LENGTH_YEARS", 15);
	check_that<DataException>(post_future_enlargement_eu_migration_multiplier >= 0);
	check_that<DataException>(post_future_enlargement_eu_migration_length_years > 0);
	std::vector<int> post_brexit_eu_migr_ref_years;
	ua.get("POST_BREXIT_EU_MIGRATION_REF_YEARS", post_brexit_eu_migr_ref_years, false);
	std::vector<double> post_brexit_eu_migr_ref_weights;
	ua.get("POST_BREXIT_EU_MIGRATION_REF_WEIGHTS", post_brexit_eu_migr_ref_weights, false);
	check_equals<size_t, size_t, DataException>(post_brexit_eu_migr_ref_years.size(), post_brexit_eu_migr_ref_weights.size(), "Number of post-Brexit reference years and weights for EU migration must be equal");
	if (post_brexit_eu_migr_ref_years.empty()) {
		post_brexit_eu_migr_ref_years.push_back(2000);
		post_brexit_eu_migr_ref_weights.push_back(1.0);
	}

	LOG_INFO() << "Read user parameters:";
	for (const auto& kv : ua.read_keys_values()) {
		LOG_INFO() << "Key = \"" << kv.first << "\", Value = \"" << kv.second << "\"";
	}

	check_that<DataException>(!(do_brexit && do_future_eu_enlargement), "Cannot model Brexit and future EU enlargement effect at the same time");

	const Ethnicity::IndexConversions ic = EthnicityClassficationsEnglandWales::get_conversions(ethnicity_classification.c_str());
	const Ethnicity::index_set_type dominant_groups({ ic.index("WHITE_BRITISH"), ic.index("IRISH") });
	Ethnicity::index_set_type eu_groups; // ethnic group set representing EU immigrants
	for (const auto& name : eu_ethnic_group_names) {
		eu_groups.insert(ic.index(name));
	}
	if (!Inclusion::is_disjoint_with(dominant_groups, eu_groups)) {
		throw DataException("EU and dominant ethnic group sets overlap");
	}
	Ethnicity::index_set_type other_groups;
	for (Ethnicity::IndexConversions::index_type i = 0; i < ic.size(); ++i) {
		if ((dominant_groups.find(i) == dominant_groups.end()) && (eu_groups.find(i) == eu_groups.end())) {
			other_groups.insert(i);
		}
	}
	LOG_INFO() << "Other ethnic groups: " << other_groups;

	// Simulation schedule
	const Schedule schedule(schedule_definition);
	std::vector<int> schedule_years(schedule.get_years<int>());
	schedule_years = Schedule::extend_back(schedule_years, max_age);

    Conception::mdistr_multi_series_type multiplicity_distros;
    {
        CSVFileReader reader(resource_dir + "multiple_births.csv");
        multiplicity_distros = ProcreationCalibrator::load_multiplicity_distros(schedule_years, reader, multiple_birth_basis);
    }
	const auto fertility_hrm_provider = get_total_fertility_rates_multipliers(resource_dir + "total_fertility_rates.csv", ethnicity_classification);
	
	std::vector<Date::year_type> census_years;
	check_that<DataException>(start_year + census_year_spacing <= end_date.year(), "At least 2 censuses must fit between start and end years");
	const Date::year_type last_census_year = std::min(end_date.year(), static_cast<Date::year_type>(2011));
	for (auto yr = start_year; yr <= last_census_year; yr = static_cast<Date::year_type>(yr + census_year_spacing)) {
		census_years.push_back(yr);
	}
	LOG_INFO() << "Census years: " << census_years;
	const std::vector<std::string> female_census_filenames(get_census_filenames(census_years, census_year_spacing, resource_dir, "census_%d_female.csv", true));
	const std::vector<std::string> male_census_filenames(get_census_filenames(census_years, census_year_spacing, resource_dir, "census_%d_male.csv", true));
	LOG_INFO() << "Loading female census numbers from files: " << female_census_filenames;
	const pop_data_vector female_census_numbers(PopulationCalibrator::load_population_numbers(female_census_filenames, DELIM, ic));
	LOG_INFO() << "Loading male census numbers from files: " << male_census_filenames;
	const pop_data_vector male_census_numbers(PopulationCalibrator::load_population_numbers(male_census_filenames, DELIM, ic));
	if (female_census_numbers[0].index() != male_census_numbers[0].index()) {
		throw DataException("Female and male census numbers have different age groups");
	}
	
	const double total_historical_population_start = calc_total_population(female_census_numbers[0], male_census_numbers[0]);
	LOG_INFO() << "Total historical population at start: " << total_historical_population_start;
	check_that<DataException>(total_historical_population_start > 0.0, "Total population in historical data must be positive");

	pop_data_vector nomigr_female_census_numbers_start;
	pop_data_vector nomigr_male_census_numbers_start;
	pop_data_vector nomigr_female_census_numbers_end;
	pop_data_vector nomigr_male_census_numbers_end;
	if (USE_SINGLE_SIMULATION_WITHOUT_MIGRATION) {
		// Load and rescale the results of a single no-migration simulation run from first to last census year
		get_single_simulation_no_migration_numbers(census_years, resource_dir, ic, total_historical_population_start,
			nomigr_female_census_numbers_start, nomigr_male_census_numbers_start,
			nomigr_female_census_numbers_end, nomigr_male_census_numbers_end);		
	} else {
		// Load and rescale the results of no-migration runs between neighbouring census years
		std::vector<double> total_historical_population_sizes(census_years.size());
		for (size_t i = 0; i < census_years.size(); ++i) {
			total_historical_population_sizes[i] = calc_total_population(female_census_numbers[i], male_census_numbers[i]);
		}
		get_multiple_simulations_no_migration_numbers(census_years, census_year_spacing, resource_dir, ic, total_historical_population_sizes,
			nomigr_female_census_numbers_start, nomigr_male_census_numbers_start,
			nomigr_female_census_numbers_end, nomigr_male_census_numbers_end);
	}
	
	//std::vector<PersonData> person_sample(load_person_data(variables_filename, persons_filename));
	std::vector<ObservedQuantity<Person>> observed_quantities;
	std::vector<ObservedQuantity<Person>> observed_quantities_ethn; // quantities observed at condensed ethnic level
	// age
	observed_quantities_ethn.push_back(ObservedQuantity<Person>("Age", [](const Person& person, const Contexts& ctx) {
		assert(person.is_alive(ctx.asof()));
		return person.age_fract(ctx.asof());
	}));
	observed_quantities.push_back(observed_quantities_ethn[0]);
	// observed history variables
	for (const auto& varname : variables_for_stats) {
		observed_quantities.push_back(ObservedQuantity<Person>::last_as_double(varname));
	}
	// BMI stats
	if (do_bmi) {
		const std::string variable_name(BMI_CATEGORY_VARIABLE_NAME); // TODO: is this necessary?
		for (bmi_cat_type i = 0; i < bmi_dim; ++i) {
			observed_quantities.push_back(ObservedQuantity<Person>(variable_name + "_" + bmi_categories[i], [i, variable_name](const Person& person, const Contexts& ctx) -> double {
				const ImmutableContext& imm_ctx = ctx.immutable_ctx();
				//TRACE() << "BMI obs: " << variable_name << ", i=" << i;
				if (person.has_history(imm_ctx, variable_name)) {
					const ImmutableHistory& history = person.history(imm_ctx, variable_name);
					if (!history.empty()) {						
						const auto k = static_cast<bmi_cat_type>(history.last_as_int(ctx.asof()));
						//TRACE() << "k=" << k << ", i=" << i << ", asof=" << ctx.asof();
						return k == i ? 1.0 : 0.0;
					}
				}
				return std::numeric_limits<double>::quiet_NaN();				
			}));
		}
	}
	// state pension age %
	if (do_spa) {
		const std::function<Date(const Person&)> date_spa_reached_calc_latest = [](const Person& person) {
			return StatePensionAge::get_date_spa_reached(person.sex(), person.date_of_birth());
		};
		const std::function<Date(const Person&)> date_spa_reached_calc_2007 = [](const Person& person) {
			return StatePensionAge2007::get_date_spa_reached(person.sex(), person.date_of_birth());
		};
		const auto date_spa_reached_calc = use_spa2007 ? date_spa_reached_calc_2007 : date_spa_reached_calc_latest;
		// SPA
		const auto reached_spa = ObservedQuantity<Person>::indicator_variable("ReachedSPA", [date_spa_reached_calc](const Person& person, const Contexts& ctx) -> bool {
			assert(person.is_alive(ctx.asof()));
			return ctx.asof() >= date_spa_reached_calc(person);
		});
		// SPA capped at 65
		const auto reached_spa65 = ObservedQuantity<Person>::indicator_variable("ReachedSPA65", [date_spa_reached_calc](const Person& person, const Contexts& ctx) -> bool {
			assert(person.is_alive(ctx.asof()));
			const Date when_im_65 = person.date_of_birth() + Period::years(65);
			const Date spa_reached = date_spa_reached_calc(person);
			return ctx.asof() >= std::min(when_im_65, spa_reached);
		});
		observed_quantities.push_back(reached_spa);
		observed_quantities_ethn.push_back(reached_spa);
		observed_quantities.push_back(reached_spa65);
		observed_quantities_ethn.push_back(reached_spa65);
	}

	// Prepare simulator
	SimulatorBuilder simulator_builder;
	simulator_builder.set_add_newborns(true); // obviously
    simulator_builder.set_initial_population_size(init_pop_size);
    const auto osr_all = std::make_shared<ObserverResultSaverSimple>(observations_filename);
    const auto osr_male = std::make_shared<ObserverResultSaverSimple>(observations_filename + "_male");
    const auto osr_female = std::make_shared<ObserverResultSaverSimple>(observations_filename + "_female");
	const auto osr_dom = std::make_shared<ObserverResultSaverSimple>(observations_filename + "_dom");
	const auto osr_male_dom = std::make_shared<ObserverResultSaverSimple>(observations_filename + "_male_dom");
	const auto osr_female_dom = std::make_shared<ObserverResultSaverSimple>(observations_filename + "_female_dom");
	const auto osr_eu = std::make_shared<ObserverResultSaverSimple>(observations_filename + "_eu");
	const auto osr_male_eu = std::make_shared<ObserverResultSaverSimple>(observations_filename + "_male_eu");
	const auto osr_female_eu = std::make_shared<ObserverResultSaverSimple>(observations_filename + "_female_eu");
	const auto osr_oth = std::make_shared<ObserverResultSaverSimple>(observations_filename + "_oth");
	const auto osr_male_oth = std::make_shared<ObserverResultSaverSimple>(observations_filename + "_male_oth");
	const auto osr_female_oth = std::make_shared<ObserverResultSaverSimple>(observations_filename + "_female_oth");
    const std::string dem_obs_prefix("demographics_");
	const auto observer_age_ranges = RateCalibrator::make_age_ranges(1, 100);
	simulator_builder.add_observer(std::make_shared<ObserverDemographicsMain>(osr_all, observer_age_ranges, schedule.nbr_dates(), dem_obs_prefix));
    simulator_builder.add_observer(std::make_shared<ObserverDemographicsImmigrants>(osr_all, observer_age_ranges, schedule.nbr_dates(), dem_obs_prefix));
    simulator_builder.add_observer(std::make_shared<ObserverDemographicsEmigrants>(osr_all, observer_age_ranges, schedule.nbr_dates(), dem_obs_prefix));
	simulator_builder.add_observer(std::make_shared<ObserverStats<Person>>(osr_all, observed_quantities, PredicateFactory::make_alive(), calc_medians));
    simulator_builder.add_observer(std::make_shared<ObserverStats<Person>>(osr_female, observed_quantities, PredicateFactory::make_sex(Sex::FEMALE, true), calc_medians));
    simulator_builder.add_observer(std::make_shared<ObserverStats<Person>>(osr_male, observed_quantities, PredicateFactory::make_sex(Sex::MALE, true), calc_medians));
	simulator_builder.add_observer(std::make_shared<ObserverStats<Person>>(osr_dom, observed_quantities_ethn, PredicateFactory::make_ethnicity(dominant_groups, true), calc_medians));
	simulator_builder.add_observer(std::make_shared<ObserverStats<Person>>(osr_female_dom, observed_quantities_ethn, PredicateFactory::make_and(PredicateFactory::make_sex(Sex::FEMALE, true), PredicateFactory::make_ethnicity(dominant_groups, true)), calc_medians));
	simulator_builder.add_observer(std::make_shared<ObserverStats<Person>>(osr_male_dom, observed_quantities_ethn, PredicateFactory::make_and(PredicateFactory::make_sex(Sex::MALE, true), PredicateFactory::make_ethnicity(dominant_groups, true)), calc_medians));
	simulator_builder.add_observer(std::make_shared<ObserverStats<Person>>(osr_eu, observed_quantities_ethn, PredicateFactory::make_ethnicity(eu_groups, true), calc_medians));
	simulator_builder.add_observer(std::make_shared<ObserverStats<Person>>(osr_female_eu, observed_quantities_ethn, PredicateFactory::make_and(PredicateFactory::make_sex(Sex::FEMALE, true), PredicateFactory::make_ethnicity(eu_groups, true)), calc_medians));
	simulator_builder.add_observer(std::make_shared<ObserverStats<Person>>(osr_male_eu, observed_quantities_ethn, PredicateFactory::make_and(PredicateFactory::make_sex(Sex::MALE, true), PredicateFactory::make_ethnicity(eu_groups, true)), calc_medians));
	simulator_builder.add_observer(std::make_shared<ObserverStats<Person>>(osr_oth, observed_quantities_ethn, PredicateFactory::make_ethnicity(other_groups, true), calc_medians));
	simulator_builder.add_observer(std::make_shared<ObserverStats<Person>>(osr_female_oth, observed_quantities_ethn, PredicateFactory::make_and(PredicateFactory::make_sex(Sex::FEMALE, true), PredicateFactory::make_ethnicity(other_groups, true)), calc_medians));
	simulator_builder.add_observer(std::make_shared<ObserverStats<Person>>(osr_male_oth, observed_quantities_ethn, PredicateFactory::make_and(PredicateFactory::make_sex(Sex::MALE, true), PredicateFactory::make_ethnicity(other_groups, true)), calc_medians));
	/*simulator_builder.add_operators(std::move(build_mortality_operators(resource_dir + "deaths_male.csv", resource_dir + "population_male.csv", schedule, max_age, PredicateFactory::make_sex(Sex::MALE))));
	simulator_builder.add_operators(build_mortality_operators(resource_dir + "deaths_female.csv", resource_dir + "population_female.csv", schedule, max_age, PredicateFactory::make_sex(Sex::FEMALE)));*/
	simulator_builder.add_operators(std::move(build_mortality_operators(resource_dir + male_mortality_filename, schedule, max_age, PredicateFactory::make_sex_shared(Sex::MALE, true))));
	simulator_builder.add_operators(build_mortality_operators(resource_dir + female_mortality_filename, schedule, max_age, PredicateFactory::make_sex_shared(Sex::FEMALE, true)));
	// cohort_fertility_rates_full.csv has fertility rates every year from 15 old, cohort_fertility_rates.csv every 5 years from 20 old
	simulator_builder.add_operators(build_conception_operators(resource_dir + cohort_fertility_rates_filename, birth_rate_basis, multiplicity_distros, fertility_hrm_provider));
	simulator_builder.add_operator(OperatorFactory::make_pregnancy(Pregnancy(), nullptr, ProcreationCalibrator::MIN_CHILDBEARING_AGE, ProcreationCalibrator::MAX_CHILDBEARING_AGE));
	simulator_builder.add_operator(OperatorFactory::make_birth(ProcreationCalibrator::MIN_CHILDBEARING_AGE, ProcreationCalibrator::MAX_CHILDBEARING_AGE));
	simulator_builder.add_operators(build_fetus_generator_operators(resource_dir + "births_sex.csv"));
	if (do_bmi) {
		simulator_builder.add_operators(build_bmi_operators(resource_dir + "BMI_CSM_ModelParams.tab", bmi_dim, bmi_min_age,
			max_age, start_date.year(), end_date.year(), start_date.month(), start_date.day(), get_bmi_ethnic_sets(ic),
			BMI_CATEGORY_VARIABLE_NAME, do_continuous_bmi ? BMI_PERCENTILE_VARIABLE_NAME : "", DELIM, true, do_continuous_bmi, bmi_thresholds, BMI_VARIABLE_NAME, max_bmi, store_bmi_percentiles_as_floats));
	}
	if (do_migration) {
		const double scale_factor = static_cast<double>(init_pop_size) / total_historical_population_start;
		if (accurate_migration) {
			if (ic.classification_name() != std::string("ONS_FULL")) {
				throw std::domain_error("Accurate migration modelling requires ONS_FULL ethnicity classification");
			}
			const std::vector<int> status_quo_years({ std::max(static_cast<Date::year_type>(census_years.back() - 5), census_years.front()) });
			const std::vector<double> status_quo_weights({ 1.0 });
			std::vector<std::unique_ptr<const MigrationCalibrator::Extender>> extenders;
			const std::vector<int> eu_ref_years = do_brexit ? post_brexit_eu_migr_ref_years : status_quo_years;
			const std::vector<double> eu_ref_weights = do_brexit ? post_brexit_eu_migr_ref_weights : status_quo_weights;
			const Ethnicity::index_set_type all(Ethnicity::index_range_to_set(ic.index_range_all()));
			if (do_brexit) {
				LOG_INFO() << "Extending migration models past " << last_census_year << " for Brexit";
				extenders.push_back(std::make_unique<MigrationCalibrator::ExtenderAgeGroup>(status_quo_years, status_quo_weights, census_years.back(), brexit_year - 1, Ethnicity::index_set_type(), 1, 1));
				LOG_DEBUG() << "Extended from " << last_census_year << " to " << (brexit_year - 1);
				// after Brexit, migration from "other" countries (Asia, Africa, etc.) may increase #helloworld
				extenders.push_back(std::make_unique<MigrationCalibrator::ExtenderAgeGroup>(status_quo_years, status_quo_weights, brexit_year, Date::MAX_YEAR, other_groups, post_brexit_other_migration_multiplier, post_brexit_other_migration_multiplier));
				LOG_DEBUG() << "Extended from " << brexit_year << " to INF for 'other' groups: " << other_groups;
				extenders.push_back(std::make_unique<MigrationCalibrator::ExtenderAgeGroup>(status_quo_years, status_quo_weights, brexit_year, Date::MAX_YEAR, dominant_groups, post_brexit_dominant_emigration_multiplier, 1));
				LOG_DEBUG() << "Extended from " << brexit_year << " to INF for 'dominant' groups: " << dominant_groups;
				// after Brexit, migration from/to EU countries goes back to eu_ref_years (e.g. 2000)
				extenders.push_back(std::make_unique<MigrationCalibrator::ExtenderAgeGroup>(eu_ref_years, eu_ref_weights, brexit_year, Date::MAX_YEAR, eu_groups, post_brexit_eu_migration_multiplier, post_brexit_eu_migration_multiplier));				
				LOG_DEBUG() << "Extended from " << brexit_year << " to INF for 'EU' groups: " << eu_groups;
			} else if (do_future_eu_enlargement) {
				LOG_INFO() << "Extending migration models past " << last_census_year << " for future EU enlargement";
				Ethnicity::index_set_type not_eu;
				for (auto idx : all) {
					if (eu_groups.find(idx) == eu_groups.end()) {
						not_eu.insert(idx);
					}
				}
				extenders.push_back(std::make_unique<MigrationCalibrator::ExtenderAgeGroup>(status_quo_years, status_quo_weights, census_years.back(), future_eu_enlargement_year - 1, Ethnicity::index_set_type(), 1, 1));
				// non EU migrates normally
				extenders.push_back(std::make_unique<MigrationCalibrator::ExtenderAgeGroup>(status_quo_years, status_quo_weights, future_eu_enlargement_year, Date::MAX_YEAR, not_eu, 1, 1));
				// EU migrates more for some time
				extenders.push_back(std::make_unique<MigrationCalibrator::ExtenderAgeGroup>(status_quo_years, status_quo_weights, future_eu_enlargement_year, future_eu_enlargement_year + post_future_enlargement_eu_migration_length_years - 1, eu_groups, post_future_enlargement_eu_migration_multiplier, post_future_enlargement_eu_migration_multiplier));
				// then it drops off to what we have now
				extenders.push_back(std::make_unique<MigrationCalibrator::ExtenderAgeGroup>(status_quo_years, status_quo_weights, future_eu_enlargement_year + post_future_enlargement_eu_migration_length_years, Date::MAX_YEAR, eu_groups, 1, 1));
			} else {
				LOG_INFO() << "Extending migration models past " << last_census_year << " for status quo";
				extenders.push_back(std::make_unique<MigrationCalibrator::ExtenderAgeGroup>(status_quo_years, status_quo_weights, census_years.back(), Date::MAX_YEAR, Ethnicity::index_set_type(), 1, 1));
			}
			simulator_builder.add_migration_generator(MigrationCalibrator::build_migration_generator("MAIN", female_census_numbers, male_census_numbers
				, nomigr_female_census_numbers_start
				, nomigr_male_census_numbers_start
				, nomigr_female_census_numbers_end
				, nomigr_male_census_numbers_end
				, census_years, MIGRATION_MID_YEAR
				, scale_factor, comigrated_child_age_limit
				, std::make_shared<const MigrantSelectorRandom>()
				, dominant_groups, extenders));			
			if (do_brexit) {
				if (post_brexit_exodus_size > 0) {
					// Affects only immigrants who arrived before Brexit.
					simulator_builder.add_migration_generator(MigrationCalibrator::build_exodus_generator("EXODUS", brexit_year, brexit_year + post_brexit_exodus_length_years, eu_groups, std::make_shared<const MigrantSelectorLatestImigrantsFirst>(), MIGRATION_MID_YEAR, post_brexit_exodus_size, is_post_brexit_exodus_size_relative, scale_factor, comigrated_child_age_limit, PredicateFactory::make_immigration_date(Date::MIN, MigrationCalibrator::make_migration_date(brexit_year, MIGRATION_MID_YEAR), false, true)));
				}
				if (post_brexit_dominant_return_multiplier > 0) {
					// select live emigrants to return
					simulator_builder.add_migration_generator(MigrationCalibrator::build_return_generator("RETURN", brexit_year, brexit_year + post_brexit_return_length_years, MIGRATION_MID_YEAR, comigrated_child_age_limit, std::make_unique<EmigrantSelector>(PredicateFactory::make_ethnicity(dominant_groups, true), Date::MIN, MigrationCalibrator::make_migration_date(brexit_year, MIGRATION_MID_YEAR), comigrated_child_age_limit), post_brexit_dominant_return_multiplier));
				}
			}
		} else {
			simulator_builder.add_migration_generator(MigrationCalibrator::build_migration_generator("MAIN", {
				std::make_pair(resource_dir + "migration_ethnic_91_01.csv", NumericalRange<int>(static_cast<int>(start_year), 2001)),
				std::make_pair(resource_dir + "migration_ethnic_01_11.csv", NumericalRange<int>(2001, 2011))
			}, ic, DELIM, MIGRATION_MID_YEAR, scale_factor, census_year_spacing, std::make_shared<const MigrantSelectorRandom>()));
		}
	} else {
		LOG_DEBUG() << "Using dummy migration generator";
		check_that<DataException>(!do_brexit, "BREXIT modeling requires accurate migration model");
		simulator_builder.add_migration_generator(std::make_shared<MigrationGeneratorDummy>());
	}
	LOG_INFO() << "Added migration generator";
	simulator_builder.add_required_features({ CommonFeatures::MORTALITY() });
	LOG_INFO() << "Added required features";
	simulator_builder.set_intermediate_observer_results_filename(observations_filename);

	// Build a simulator
	Simulator simulator(simulator_builder.build(Contexts(std::make_shared<ImmutableContext>( schedule, ic), std::make_shared<MutableContext>())));

	if (only_calibration) {
		LOG_INFO() << "Exiting after calibration";
		return;
	}

	// Bootstrap a population	
	Population population("MAIN");
	simulator.initialise_population(InitialiserGenerations(PopulationCalibrator::make_generations(female_census_numbers[0], male_census_numbers[0], start_year, max_age, ic, start_date)), population);

	// Run the simulation!
	simulator.run(population);
	simulator.save_observer_results();

	LOG_INFO() << "Simulation finished";
}


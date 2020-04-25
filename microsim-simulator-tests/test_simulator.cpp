/*
(C) Averisera Ltd 2017
*/
#include <gtest/gtest.h>
#include "microsim-simulator/contexts.hpp"
#include "microsim-simulator/common_features.hpp"
#include "microsim-simulator/immutable_context.hpp"
#include "microsim-simulator/operator_factory.hpp"
#include "microsim-simulator/person.hpp"
#include "microsim-simulator/predicate_factory.hpp"
#include "microsim-simulator/simulator.hpp"
#include "microsim-simulator/initialiser/generation.hpp"
#include "microsim-simulator/initialiser/initialiser_generations.hpp"
#include "microsim-simulator/migration/emigrant_selector.hpp"
#include "microsim-simulator/migration/migration_generator_dummy.hpp"
#include "microsim-simulator/migration/migration_generator_model.hpp"
#include "microsim-simulator/migration/migration_generator_return.hpp"
#include "microsim-simulator/migration/migrant_selector_random.hpp"
#include "microsim-simulator/observer/observed_quantity.hpp"
#include "microsim-simulator/observer/observer_demographics_main.hpp"
#include "microsim-simulator/observer/observer_demographics_immigrants.hpp"
#include "microsim-simulator/observer/observer_demographics_emigrants.hpp"
#include "microsim-simulator/observer/observer_stats.hpp"
#include "microsim-simulator/observer/observer_result_saver_simple.hpp"
#include "microsim-calibrator/rate_calibrator.hpp"
#include "microsim-core/anchored_hazard_curve.hpp"
#include "microsim-core/conception.hpp"
#include "microsim-core/hazard_curve_factory.hpp"
#include "microsim-core/hazard_curve.hpp"
#include "microsim-core/schedule_definition.hpp"
#include "core/generic_distribution_enumerated.hpp"

using namespace averisera;
using namespace averisera::microsim;

/** Mock ethnic group classification for testing */
struct EthnicityMock {
	enum class Group : PersonAttributes::ethnicity_t {
		BLACK = 0,
		WHITE,
		SIZE /**< Use it to get the number of other values */
	};

	/** Number of groups */
	static const size_t SIZE = static_cast<size_t>(Group::SIZE);

	/** Name of the classification scheme */
	static const char* const CLASSIFICATION_NAME;

	/** Names of groups */
	static const std::array<const char*, SIZE + 1> NAMES;
};

const char* const EthnicityMock::CLASSIFICATION_NAME = "MOCK";

const std::array<const char*, EthnicityMock::SIZE + 1> EthnicityMock::NAMES = {
	"BLACK",
	"WHITE",
	"" /**< SIZE */
};

std::ostream& operator<<(std::ostream& os, EthnicityMock::Group group) {
	os << Ethnicity::get_name<EthnicityMock>(group);
	return os;
}

static const uint8_t ETHN_BLACK = 0;
static const uint8_t ETHN_WHITE = 1;

std::unique_ptr<HazardCurve> build_mortality_curve() {
	std::vector<double> times;
	std::vector<double> probs;
	times.push_back(1.0);
	probs.push_back(0.001);

	times.push_back(40);
	probs.push_back(0.05);

	times.push_back(50);
	probs.push_back(0.1);

	times.push_back(60);
	probs.push_back(0.1);

	times.push_back(70);
	probs.push_back(0.2);

	times.push_back(80);
	probs.push_back(0.3);

	times.push_back(90);
	probs.push_back(0.5);

	times.push_back(95);
	probs.push_back(0.8);

	times.push_back(100);
	probs.push_back(0.99);

	return HazardCurveFactory::PIECEWISE_CONSTANT()->build(times, probs, true);
}

std::vector<Generation> build_initial_population_state() {
	const std::vector<PersonAttributes> pa_values({ PersonAttributes(Sex::FEMALE, ETHN_BLACK), PersonAttributes(Sex::FEMALE, ETHN_WHITE), PersonAttributes(Sex::MALE, ETHN_BLACK), PersonAttributes(Sex::MALE, ETHN_WHITE) });
	std::vector<Generation> generations;
	generations.push_back(Generation(Date(1900, 1, 1), Date(1930, 1, 1), PersonAttributesDistribution(GenericDistributionEnumerated<PersonAttributes>::from_unsorted(pa_values, std::vector<double>({ 0.0, 0.6, 0.0, 0.4 }))), 0.1));
	generations.push_back(Generation(Date(1930, 1, 1), Date(1950, 1, 1), PersonAttributesDistribution(GenericDistributionEnumerated<PersonAttributes>::from_unsorted(pa_values, std::vector<double>({ 0.06, 0.435, 0.08, 0.425 }))), 0.25));
	generations.push_back(Generation(Date(1950, 1, 1), Date(1970, 1, 1), PersonAttributesDistribution(GenericDistributionEnumerated<PersonAttributes>::from_unsorted(pa_values, std::vector<double>({ 0.1, 0.4, 0.1, 0.4 }))), 0.35));
	generations.push_back(Generation(Date(1970, 1, 1), Date(1990, 1, 1), PersonAttributesDistribution(GenericDistributionEnumerated<PersonAttributes>::from_unsorted(pa_values, std::vector<double>({ 0.15, 0.35, 0.15, 0.35 }))), 0.3));
	return generations;
}

std::shared_ptr<const MigrationGenerator> build_migration_generator_model(unsigned int lower_age_limit) {
	std::vector<MigrationGeneratorModel::pred_model_pair> models;
	models.push_back(std::make_pair(PredicateFactory::make_ethnicity(Ethnicity::index_set_type({ ETHN_BLACK }), true), MigrationModel(MigrationModel::MigrationRatePerAnnum(0.0, 100.0))));
	models.push_back(std::make_pair(PredicateFactory::make_ethnicity(Ethnicity::index_set_type({ ETHN_WHITE }), true), MigrationModel(MigrationModel::MigrationRatePerAnnum(-0.005, 0.0))));
	return std::make_shared<MigrationGeneratorModel>("MAIN", std::move(models), lower_age_limit, std::make_shared<const MigrantSelectorRandom>());
}

// test run a simple simulator
TEST(Simulator, Simples) {
	const Date start_date(1990, 1, 1);
	const Date end_date(1995, 1, 1);
	const size_t initial_pop_size = 10000;
	const bool add_newborns = true;
	const unsigned int min_childbearing_age = 15;
	const unsigned int max_childbearing_age = 45;
	const unsigned int zero_fertility_period_months = 3;
	const Period zero_fertility_period(PeriodType::MONTHS, zero_fertility_period_months);
	const Period frequency(PeriodType::MONTHS, 6);
	const unsigned int migration_child_age_limit = 15;

	const ScheduleDefinition schedule_definition(start_date, end_date, frequency);
	const Schedule schedule(schedule_definition);
	const std::shared_ptr<ImmutableContext> immutable_context(new ImmutableContext(schedule, Ethnicity::IndexConversions::build<EthnicityMock>()));
	const std::shared_ptr<MutableContext> mutable_context(new MutableContext());
	Contexts ctx(immutable_context, mutable_context);
	std::vector<std::shared_ptr<Operator<Person>>> person_operators;

	// procreation
	person_operators.push_back(OperatorFactory::make_birth(min_childbearing_age, max_childbearing_age));
	person_operators.push_back(OperatorFactory::make_fetus_generator_simple(min_childbearing_age, max_childbearing_age, 0.52)); // slightly more girls than boys
	person_operators.push_back(OperatorFactory::make_pregnancy(Pregnancy(), nullptr, min_childbearing_age, max_childbearing_age));
	const double conc_dt = 0.75 + static_cast<double>(zero_fertility_period_months) / 12.0;
	const double conc_hazard_black = RateCalibrator::hazard_rate_from_average_occurrences(max_childbearing_age - min_childbearing_age + 1, 3.4, conc_dt);
	const double conc_hazard_white = RateCalibrator::hazard_rate_from_average_occurrences(max_childbearing_age - min_childbearing_age + 1, 1.7, conc_dt);
	const auto daycount = Daycount::DAYS_365();
	person_operators.push_back(OperatorFactory::make_conception(Conception(AnchoredHazardCurve::build(start_date, daycount, HazardCurveFactory::make_flat(conc_hazard_black))),
		std::vector<std::shared_ptr<const RelativeRisk<Person>>>(),
		PredicateFactory::make_ethnicity(Ethnicity::index_set_type({ ETHN_BLACK }), true),
		nullptr, nullptr, min_childbearing_age, max_childbearing_age, zero_fertility_period));
	person_operators.push_back(OperatorFactory::make_conception(Conception(AnchoredHazardCurve::build(start_date, daycount, HazardCurveFactory::make_flat(conc_hazard_white))),
		std::vector<std::shared_ptr<const RelativeRisk<Person>>>(),
		PredicateFactory::make_ethnicity(Ethnicity::index_set_type({ETHN_WHITE}), true),
		nullptr, nullptr, min_childbearing_age, max_childbearing_age, zero_fertility_period));

	// mortality
	person_operators.push_back(OperatorFactory::make_mortality(HazardModel(AnchoredHazardCurve::build(start_date, daycount, build_mortality_curve())), std::vector<std::shared_ptr<const RelativeRisk<Person>>>(), PredicateFactory::make_alive(), nullptr, true));

	

	// migration
	std::vector<std::shared_ptr<const MigrationGenerator>> migration_generators;
	migration_generators.push_back(std::make_shared<MigrationGeneratorReturn>("RETURN", Date(1993, 1, 1), Date(1995, 1, 1), 0.2, migration_child_age_limit, std::make_unique<EmigrantSelector>(PredicateFactory::make_ethnicity(Ethnicity::index_set_type({ ETHN_WHITE }), true), Date::MIN, Date::MAX, migration_child_age_limit)));
	migration_generators.push_back(build_migration_generator_model(migration_child_age_limit));
	//migration_generators.push_back(std::make_shared<MigrationGeneratorDummy>());

	// observers
    const auto osr_all = std::make_shared<ObserverResultSaverSimple>("", "test_observations.txt");
	std::vector<std::shared_ptr<Observer>> observers;
    const auto age_ranges = ObserverDemographics::age_ranges_type({
            ObserverDemographics::age_range_type(0, 20),
                ObserverDemographics::age_range_type(20, 40),
                ObserverDemographics::age_range_type(40, 60),
                ObserverDemographics::age_range_type(60, 80),
                ObserverDemographics::age_range_type(80, 120)
                });
	observers.push_back(std::make_shared<ObserverDemographicsMain>(osr_all, age_ranges, schedule.nbr_dates()));
    observers.push_back(std::make_shared<ObserverDemographicsEmigrants>(osr_all, age_ranges, schedule.nbr_dates()));
    observers.push_back(std::make_shared<ObserverDemographicsImmigrants>(osr_all, age_ranges, schedule.nbr_dates()));
    std::vector<ObservedQuantity<Person>> observed_quantities;
    observed_quantities.push_back(ObservedQuantity<Person>("Age", [](const Person& person, const Contexts& ctx) {            
		assert(person.is_alive(ctx.asof()));
        return person.age_fract(ctx.asof());
	}));
    const auto osr_male = std::make_shared<ObserverResultSaverSimple>("", "test_observations_male.txt");
    const auto osr_female = std::make_shared<ObserverResultSaverSimple>("", "test_observations_female.txt");
	const bool calc_median = true;
    observers.push_back(std::make_shared<ObserverStats<Person>>(osr_all, observed_quantities, PredicateFactory::make_alive(), calc_median));
    observers.push_back(std::make_shared<ObserverStats<Person>>(osr_female, observed_quantities, PredicateFactory::make_sex(Sex::FEMALE, true), calc_median));
    observers.push_back(std::make_shared<ObserverStats<Person>>(osr_male, observed_quantities, PredicateFactory::make_sex(Sex::MALE, true), calc_median));

	ctx.immutable_ctx().collect_history_requirements(person_operators);

	Simulator simulator(std::move(ctx), std::move(person_operators), std::move(observers), std::move(migration_generators),
		add_newborns, initial_pop_size, { CommonFeatures::MORTALITY() }, std::string());

	// initialise population
	const InitialiserGenerations initialiser(build_initial_population_state());
	Population population("MAIN");
	simulator.initialise_population(initialiser, population);

	// Run the simulation!
	simulator.run(population);
	simulator.save_observer_results();

	const size_t final_size = population.persons().size();
	ASSERT_GT(static_cast<double>(final_size), 1.05 * static_cast<double>(initial_pop_size));
	std::cout << "Population increased from " << initial_pop_size << " to " << final_size;
	ASSERT_NE(mutable_context->emigrants().size(), 0);
}

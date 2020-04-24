#pragma once
#include "calibration_types.hpp"
#include "core/data_frame.hpp"
#include "core/numerical_range.hpp"
#include "core/time_series.hpp"
#include <string>
#include <utility>
#include <vector>

namespace averisera {
	class CSVFileReader;
	template <class T, class V> class TimeSeries;
	namespace microsim {
		class EmigrantSelector;
		class MigrantSelector;
		class MigrationModel;		
		class Person;
		template <class A> class Predicate;
		class MigrationGenerator;

		namespace MigrationCalibrator {
			using namespace CalibrationTypes;
			typedef std::pair<std::unique_ptr<const Predicate<Person>>, MigrationModel> pred_model_pair;

			const std::string& BASE_COL(); /*! Column name with base level */
			const std::string& MIGRATION_COL(); /*! Column name with net migration level */
			const std::string& MODEL_COL(); /*! Column name with migration model specification */			

			DataFrame<std::string, Ethnicity::index_set_type> load_net_migration(CSVFileReader& reader, const Ethnicity::IndexConversions& ic);

			Date make_migration_date(int year, bool mid_year);

			std::vector<pred_model_pair> load_migration_models(const std::vector < std::pair<std::string, NumericalRange<int>>>& filenames_for_periods, const Ethnicity::IndexConversions& ic, CSV::Delimiter delim, bool mid_year);

			/*!\param scale_factor Rescale absolute migration rates by this factor 
			\param comigrated_child_age_limit Mothers always migrate with children which are less than comigrated_child_age_limit years old (rounded down)
			*/
			std::unique_ptr<MigrationGenerator> build_migration_generator(std::string&& name, const std::vector < std::pair<std::string, NumericalRange<int>>>& filenames_for_periods, const Ethnicity::IndexConversions& ic, CSV::Delimiter delim, bool mid_year, double scale_factor, unsigned int comigrated_child_age_limit, std::shared_ptr<const MigrantSelector> emigrant_selector);

			typedef DataFrame<std::string, Ethnicity::index_set_type> data_type;
			struct MigrationDataSet {
				data_type data; // migration data
				std::vector<std::string> types;
				NumericalRange<int> data_year_range; // year range over which the data have been gathered

				MigrationDataSet();
				MigrationDataSet(data_type&& n_data, std::vector<std::string>&& n_types, NumericalRange<int>&& n_data_year_range);

				MigrationDataSet drop_ethnic_set(const Ethnicity::index_set_type& set) const;

				/*! Multiply data in place by a scalar */
				MigrationDataSet& operator*=(double x);

				/*! Multiply data by a scalar */
				MigrationDataSet operator*(double x) const;

				MigrationDataSet& operator+=(const MigrationDataSet& other);
			};

			/*! Extends the generated models into the future*/
			class Extender {
			public:
				virtual ~Extender();

				/*! Use the generated migration data to extend the simulation beyond census years. */
				virtual void extend(const TimeSeries<int, std::unordered_map<age_group_type, MigrationDataSet>>& data_maps, const std::shared_ptr<const Predicate<Person>>& other_pred, bool mid_year, std::vector<pred_model_pair>& result, unsigned int min_age) const = 0;
			};

			/*!
			\param female_actual Female actual census numbers for years Y1, ..., YN
			\param male_actual
			\param female_nomigr_start Female census numbers for years Y1, ..., YN-1 simulated without migration (for start of each inter-census period)
			\param male_nomigr_start Male census numbers for years Y1, ..., YN-1 simulated without migration (for start of each inter-census period)
			\param female_nomigr_end Female census numbers for years Y2, ..., YN simulated without migration (for end of each inter-census period)
			\param male_nomigr_end Male census numbers for years Y2, ..., YN simulated without migration (for end of each inter-census period)
			\param census_years Census years Y1, ..., YN
			\param scale_factor Rescale absolute migration rates by this factor 
			\param comigrated_child_age_limit Mothers always migrate with children which are less than comigrated_child_age_limit years old (rounded down)
			\param dominant_ethnic_groups Set of ethnic groups which dominate the population and mostly migrate out. Their negative migration trends are modelled as RELATIVE.
			\param extenders Define how migration rates are extended beyond the range of census years
			*/
			std::unique_ptr<MigrationGenerator> build_migration_generator(std::string&& name, 
				const std::vector<pop_data_type>& female_actual,
				const std::vector<pop_data_type>& male_actual, 
				const std::vector<pop_data_type>& female_nomigr_start, 
				const std::vector<pop_data_type>& male_nomigr_start, 
				const std::vector<pop_data_type>& female_nomigr_end,
				const std::vector<pop_data_type>& male_nomigr_end,
				const std::vector<Date::year_type>& census_years, 
				bool mid_year, 
				double scale_factor, 
				unsigned int comigrated_child_age_limit, 
				std::shared_ptr<const MigrantSelector> emigrant_selector, 
				const Ethnicity::index_set_type& dominant_ethnic_groups, 
				const std::vector<std::unique_ptr<const Extender>>& extenders);

			/*! Applies the same migration model to each age group every year */
			class ExtenderAgeGroup: public Extender {
			public:
				/*! 
				\param src_years Source years
				\param src_weights Source year weights
				\param from_year First target year (inclusive)
				\param to_year Last target year (inclusive)
				\param ethn_groups If not empty, extend only for the ethnic groups in this set 
				\param multiplier_emigrants Multiplier for numbers of emigrants, >= 0
				\param multiplier_immigrants Multiplier for numbers of immigrants, >= 0
				\throw std::out_of_range If multiplier_emigrants < 0 or multiplier_immigrants < 0
				*/
				ExtenderAgeGroup(const std::vector<int>& src_years, const std::vector<double>& src_weights, int from_year, 
					int to_year, const Ethnicity::index_set_type& ethn_groups, double multiplier_emigrants, double multiplier_immigrants);

				/*!
				\param src_years Source years
				\param src_weights Source year weights
				Each group will be multiplied by the value of the time series corresponding to the median age of the group.
				Time series will be padded forward (for ages after the first one) or backward (for ages before the first one).
				*/
				ExtenderAgeGroup(const std::vector<int>& src_years, const std::vector<double>& src_weights, int from_year, int to_year, const Ethnicity::index_set_type& ethn_groups, const TimeSeries<unsigned int, double>& multiplier_emigrants_by_age, const TimeSeries<unsigned int, double>& multiplier_immigrants_by_age);

				void extend(const TimeSeries<int, std::unordered_map<age_group_type, MigrationDataSet>>& data_maps, const std::shared_ptr<const Predicate<Person>>& other_pred, bool mid_year, std::vector<pred_model_pair>& result, unsigned int min_age) const override;
			private:
				std::vector<int> src_years_;
				std::vector<double> src_weights_;
				int from_year_;
				int to_year_;
				Ethnicity::index_set_type ethn_groups_;
				TimeSeries<unsigned int, double> multiplier_emigrants_by_age_;
				TimeSeries<unsigned int, double> multiplier_immigrants_by_age_;

				MigrationDataSet filter_ethnic_groups(MigrationDataSet mds) const;
			};			

			/*!
			\param year_from Inclusive
			\param year_to Exclusive
			*/
			std::unique_ptr<MigrationGenerator> build_exodus_generator(std::string&& name, int year_from, int year_to, 
				const Ethnicity::index_set_type& covered_groups, std::shared_ptr<const MigrantSelector> emigrant_selector, bool mid_year, double exodus_size, bool is_relative, double scale_factor, unsigned int comigrated_child_age_limit, std::shared_ptr<const Predicate<Person>> exodus_predicate);

			/*!
			\param return_year_from Inclusive
			\param return_year_to Exclusive
			\param mid_year If true, cover dates from 1 July to 1 July, else from 1 January to 1 January.
			\param returning_children_age_limit Children below that age return with their mother only (they are not selected).
			\param emigrant_selector Emigrant selector.
			\param returning_fraction Fraction of emigrants returning in [0, 1].
			*/
			std::unique_ptr<MigrationGenerator> build_return_generator(std::string&& name, int return_year_from, 
				int return_year_to, bool mid_year, unsigned int returning_children_age_limit, std::unique_ptr<EmigrantSelector>&& emigrant_selector, double returning_fraction);
		}
	}
}

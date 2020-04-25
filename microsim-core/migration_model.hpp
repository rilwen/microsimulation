#pragma once
#include "sex.hpp"
#include "core/time_series.hpp"
#include <iosfwd>
#include <utility>

namespace averisera {
	namespace microsim {
		/** Different pattern by age, sex and ethnic group */
		class MigrationModel {
		public:
			/** Migration rate per annum. 
			dN/dt = relative * N + absolute
			*/
			class MigrationRatePerAnnum {
			public:												
				MigrationRatePerAnnum(double rel = 0, double abs = 0);

				double relative() const {
					return relative_;
				}

				double absolute() const {
					return absolute_;
				}
				
				/** Rescale the rate to different base population by factor X */
				MigrationRatePerAnnum& scale_base(double x) {
					absolute_ *= x;					
					return *this;
				}				
			private:
				double relative_;
				double absolute_;
			};

			typedef TimeSeries<Date, MigrationRatePerAnnum> time_dependent_migration_rate;
			
			/** Given x(0), calculate x(t) - x(0), where dx/dt = relative * x + absolute */
			static double calculate_migration(double x0, double t, const MigrationRatePerAnnum& r);

			/** Reverse the calculation given the migration data and assumed absolute rate 			
			@throw std::out_of_range If x0 < 0 or xt < 0. If x0 == 0 and xt != 0. If t <= 0. */
			static MigrationRatePerAnnum calibrate_rate(double x0, double xt, double t, double absolute_rate);

			MigrationModel();			
			
			/** @throw std::out_of_range If base_pop_size < 0*/
			MigrationModel(time_dependent_migration_rate&& migration_rates);

			/** Constant rate of migration */
			MigrationModel(MigrationRatePerAnnum rate);

			MigrationModel(MigrationModel&& other);

			MigrationModel& operator=(MigrationModel&& other);

			MigrationRatePerAnnum get_rate(Date date) const;

			/** Convert date range to dt value */
			static double calc_dt(Date from, Date to);

			/** Calculate migration between two dates
			@param x0 Starting population */
			double calculate_migration(Date from, Date to, double x0) const;

			MigrationModel& scale_base(double x) {
				for (auto vit = migration_rates_.values_begin(); vit != migration_rates_.values_end(); ++vit) {
					vit->scale_base(x);
				}
				return *this;
			}
		private:
			time_dependent_migration_rate migration_rates_;			
		};

		std::ostream& operator<<(std::ostream& os, const MigrationModel::MigrationRatePerAnnum& rate);
	}
}

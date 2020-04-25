#ifndef __AVERISERA_MICROSIM_CONCEPTION_H
#define __AVERISERA_MICROSIM_CONCEPTION_H

#include "core/time_series.hpp"
#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <boost/format.hpp>

namespace averisera {
    template <class T> class GenericDistribution;
    
    namespace microsim {
        class AnchoredHazardCurve;
        class HazardModel;
        
        /** Conception model. Describes the time of conception and pregnancy multiplicity. Genders should be assigned separately.
         */
        class Conception {
        public:
            typedef uint8_t multiplicity_type;
			typedef GenericDistribution<multiplicity_type> multiplicity_distr_type;
			typedef TimeSeries<double, std::shared_ptr<const multiplicity_distr_type>> mdistr_series_type;			
			typedef TimeSeries<int, mdistr_series_type> mdistr_multi_series_type;

			/** Construct the model
              @param[in] conception_curve Describes the risk of conception at various dates.
              @param[in] multiplicity_distros TimeSeries (indexed by calendar year) of TimeSeries (indexed by age at time of conception) of distributions of pregnancy multiplicities (single, twins, triplets...), forward-looking. Distributions have to start at 1 or higher.
              @param[in] move_to_dob Whether the conception risk curve should be moved to the date of birth of the woman
              @throw If conception_curve is null. If multiplicity_distros is empty. If multiplicity_distros.first_time() is not 0. If d is null or d.lower_bound() == 0 for any d in multiplicity_distros.
             */
            Conception(std::shared_ptr<const AnchoredHazardCurve> conception_curve, mdistr_multi_series_type&& multiplicity_distros);

			/** Construct the model using a single age-dependent multiplicity distribution for all times.
			*/
			Conception(std::shared_ptr<const AnchoredHazardCurve> conception_curve, mdistr_series_type&& multiplicity_distro)
				: Conception(conception_curve, mdistr_multi_series_type(2000, std::move(multiplicity_distro))) {}

			/** Construct the model using a single multiplicity distribution for all times and ages.
			*/
			Conception(std::shared_ptr<const AnchoredHazardCurve> conception_curve, const std::shared_ptr<const multiplicity_distr_type>& multiplicity_distro)
				: Conception(conception_curve, mdistr_series_type(0.0, multiplicity_distro)) {}

			/** Construct the model with only single pregnancies */
			Conception(std::shared_ptr<const AnchoredHazardCurve> conception_curve);

            /** Move constructor */
            Conception(Conception&& other) noexcept;
            
            Conception(const Conception&) = default;
            Conception& operator=(const Conception&) = default;

            /** Return HazardModel for conception
             */
            HazardModel hazard_model(Date date_of_birth) const;

            bool move_to_date_of_birth() const {
                return _move_to_dob;
            }

            /** Get multiplicity distribution for given age in given year
              @param age >= 0
              @throw std::out_of_range If age is negative
            */
            const GenericDistribution<multiplicity_type>& multiplicity(int year, double age) const {
                if (age < 0) {
                    throw std::out_of_range(boost::str(boost::format("Conception: negative age %g") % age));
                }
				const auto& md = _multiplicity_distros.padded_value(year);
                const auto* valptr = md.last_value(age);
                assert(valptr);
                assert(*valptr);
                return **valptr;
            }
        private:
            std::shared_ptr<const AnchoredHazardCurve> _conception_curve;
			mdistr_multi_series_type _multiplicity_distros;
            bool _move_to_dob;
        };
    }
}

#endif // __AVERISERA_MICROSIM_CONCEPTION_H

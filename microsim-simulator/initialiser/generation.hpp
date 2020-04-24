#pragma once
#include "core/dates_fwd.hpp"
#include "core/date_distribution_from_real.hpp"
#include "core/daycount.hpp"
#include "core/distribution_linear_interpolated.hpp"
#include "microsim-core/person_attributes_distribution.hpp"
#include <memory>

namespace averisera {
    template <class T> class GenericDistribution;
    namespace microsim {
        /*! Description of a generation */
        class Generation {
        public:
            /*! Construct generation with uniform date of birth distribution inside [begin, end) range.
              \param begin First date of birth of the generation (inclusive)
              \param end date End date of birth of the generation (exclusive)
              \param attrib_distr PersonAttributes distribution
              \param prob Probability of belonging to this generation
              \throw std::domain_error If end <= begin or prob outside [0, 1]
            */
            Generation(Date begin, Date end, PersonAttributesDistribution attrib_distr, double prob);

            /*! Construct generation, calculating the conditional DOB distribution from the global DOB distribution (across all generations)
              \param begin First date of birth of the generation (inclusive)
              \param end date End date of birth of the generation (exclusive)
              \param attrib_distr PersonAttributes distribution
              \param global_dob_distr Global DOB distribution
              \throw std::domain_error If end <= begin, global_dob_distr is null
            */
            Generation(Date begin, Date end, PersonAttributesDistribution attrib_distr, std::shared_ptr<const GenericDistribution<Date>> global_dob_distr);

            /*! Construct generation with specified date of birth distribution inside [begin, end) range.
              \param begin First date of birth of the generation (inclusive)
              \param end date End date of birth of the generation (exclusive)
              \param attrib_distr PersonAttributes distribution
              \param prob Probability of belonging to this generation
              \param dob_distr DOB distribution *within* this generation
              \throw std::domain_error If end <= begin, dob_distr is null or prob outside [0, 1]
            */
            Generation(Date begin, Date end, PersonAttributesDistribution attrib_distr, double prob, std::shared_ptr<const GenericDistribution<Date>> dob_distr);

            Date begin() const {
                return _begin;
            }
                
            Date end() const {
                return _end;
            }
                
            const PersonAttributesDistribution& attrib_distr() const {
                return _attrib_distr;
            }

            /*! Probability of belonging to this generation */
            double prob() const {
                return _prob;
            }

            const GenericDistribution<Date>& dob_distr() const {
                return *_dob_distr;
            }
        private:
            void validate() const;
                
            Date _begin;
            Date _end;
            PersonAttributesDistribution _attrib_distr;
            double _prob;
            std::shared_ptr<const GenericDistribution<Date>> _dob_distr;
        };
    }
}

#ifndef __AVERISERA_MICROSIM_PERSON_ATTRIBUTES_DISTRIBUTION_H
#define __AVERISERA_MICROSIM_PERSON_ATTRIBUTES_DISTRIBUTION_H

#include <memory>

namespace averisera {
    template <class T> class GenericDistribution;
    class RNG;
    
    namespace microsim {
        class PersonAttributes;

        /*! Allows drawing PersonAttributes values */
        class PersonAttributesDistribution {
        public:
            /*! Construct from a distribution which draws values converted using static_cast to values from PersonAttributes.
              \throw std::domain_error If distr is null or has dimension different than number of values in PersonAttributes.
            */
            PersonAttributesDistribution(std::shared_ptr<const GenericDistribution<PersonAttributes>> distr);

            /*! Draw a random set of attributes */
            PersonAttributes draw(RNG& rng) const;			
        private:
            std::shared_ptr<const GenericDistribution<PersonAttributes>> _distr;
        };
    }
}

#endif // __AVERISERA_MICROSIM_PERSON_ATTRIBUTES_DISTRIBUTION_H


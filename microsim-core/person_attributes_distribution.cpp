#include "person_attributes_distribution.hpp"
#include "person_attributes.hpp"
#include "core/generic_distribution.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        PersonAttributesDistribution::PersonAttributesDistribution(std::shared_ptr<const GenericDistribution<PersonAttributes>> distr)
            : _distr(distr) {
            if (!distr) {
                throw std::domain_error("PersonAttributesDistribution: null distribution");
            }            
        }

        PersonAttributes PersonAttributesDistribution::draw(RNG& rng) const {
            return _distr->random(rng);			
        }
    }
}

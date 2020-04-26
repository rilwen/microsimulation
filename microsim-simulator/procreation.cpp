// (C) Averisera Ltd 2014-2020
#include "procreation.hpp"
#include "feature.hpp"

namespace averisera {
    namespace microsim {
        namespace Procreation {
            const std::string& CONCEPTION() {
                static const std::string name("CONCEPTION");
                return name;
            }
            
            const std::string& PREGNANCY_EVENT() {
                static const std::string name("PREGNANCY_EVENT");
                return name;
            }

            const Feature& CONCEPTION_FEATURE() {
                static const Feature conception_feature(CONCEPTION());
                return conception_feature;
            }

            const Feature& CHILD_GENERATION() {
                static const Feature child_generation("CHILD_GENERATION");
                return child_generation;
            }
            
            const Feature& PREGNANCY_FEATURE() {
                static const Feature pregnancy_feature("PREGNANCY");
                return pregnancy_feature;
            }

            const Feature& BIRTH_FEATURE() {
                static const Feature feature("BIRTH");
                return feature;
            }
        }
    }
}

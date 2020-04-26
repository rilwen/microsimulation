// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MICROSIM_PROCREATION_HPP
#define __AVERISERA_MICROSIM_PROCREATION_HPP

#include <string>

namespace averisera {
    namespace microsim {
        class Feature;
        
        /** Common settings related to procreation modelling */
        namespace Procreation {
            /** Variable for conception event details */
            const std::string& CONCEPTION();

            /** Variable for pregnancy event */
            const std::string& PREGNANCY_EVENT();

			/** Conception modelling feature */
            const Feature& CONCEPTION_FEATURE();

            /** Child generation feature */
            const Feature& CHILD_GENERATION();

            /** Pregnancy modelling feature */
            const Feature& PREGNANCY_FEATURE();

            /** Birth feature */
            const Feature& BIRTH_FEATURE();
        }
    }
}

#endif // __AVERISERA_MICROSIM_PROCREATION_HPP

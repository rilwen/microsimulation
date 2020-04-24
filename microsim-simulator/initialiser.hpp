#ifndef __AVERISERA_MICROSIM_INITIALISER_H
#define __AVERISERA_MICROSIM_INITIALISER_H

#include <cstdlib>

namespace averisera {
    namespace microsim {
        struct ActorData;
        class Contexts;
        class ImmutableContext;
        struct PopulationData;
        
        /*! Initialises PopulationData to cold-start a simulation.
         */
        class Initialiser {
        public:
            typedef size_t pop_size_t;
            virtual ~Initialiser();

            /*! Initialise a population. Generate data objects with correct IDs.
              \param[in] total_size Total requested size of the population
              \param[in] ctx Contexts. The *last* date in ctx should be the starting date of the main simulation.
              \throw std::logic_error If initialiser procedure malfunctions due to programmer error.
             */
            virtual PopulationData initialise(pop_size_t total_size, const Contexts& ctx) const = 0;                   
        };
    }
}

#endif // __AVERISERA_MICROSIM_INITIALISER_H

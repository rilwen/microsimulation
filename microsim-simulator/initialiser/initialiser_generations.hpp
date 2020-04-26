// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MICROSIM_INITIALISER_GENERATIONS_H
#define __AVERISERA_MICROSIM_INITIALISER_GENERATIONS_H

#include "../initialiser.hpp"
#include "generation.hpp"
#include "core/discrete_distribution.hpp"
#include <vector>

namespace averisera {
    namespace microsim {
        struct PopulationData;
        
        /** Initialiser which creates every generation separately */
        class InitialiserGenerations: public Initialiser {
        public:
            /** @throw std::domain_error If generations have overlapping date ranges or no generations given */
            InitialiserGenerations(const std::vector<Generation>& generations);

            /** @throw std::domain_error If generations have overlapping date ranges or no generations given */
            InitialiserGenerations(std::vector<Generation>&& generations);

            PopulationData initialise(pop_size_t total_size, const Contexts& ctx) const override;
        private:
            std::vector<Generation> _generations;
            DiscreteDistribution _generation_distribution;

            void init();
        };
    }
}

#endif // __AVERISERA_MICROSIM_INITIALISER_GENERATIONS_H

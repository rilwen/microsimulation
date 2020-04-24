#include "../contexts.hpp"
#include "generation.hpp"
#include "initialiser_generations.hpp"
#include "../population_data.hpp"
#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        InitialiserGenerations::InitialiserGenerations(const std::vector<Generation>& generations)
            : _generations(generations) {
            init();
        }

        InitialiserGenerations::InitialiserGenerations(std::vector<Generation>&& generations)
            : _generations(std::move(generations)) {
            init();
        }

        PopulationData InitialiserGenerations::initialise(const pop_size_t total_size, const Contexts& ctx) const {
            PopulationData data;
            data.persons.reserve(total_size);
            MutableContext& mut_ctx = ctx.mutable_ctx();
            RNG& rng = mut_ctx.rng();
            for (pop_size_t i = 0; i < total_size; ++i) {
                const int gen_idx = _generation_distribution.random(rng);
                assert(static_cast<size_t>(gen_idx) < _generations.size());
                const Generation& generation = _generations[gen_idx];
                PersonData person_data;
                person_data.date_of_birth = generation.dob_distr().random(rng);
                person_data.attributes = generation.attrib_distr().draw(rng);
                person_data.id = mut_ctx.gen_id();
                data.persons.push_back(std::move(person_data));

            }
            return data;
        }

        void InitialiserGenerations::init() {
            if (_generations.empty()) {
                throw std::domain_error("InitialiserGenerations: no generations");
            }
            std::sort(_generations.begin(), _generations.end(), [](const Generation& g1, const Generation& g2) {
                    if (g1.end() <= g2.begin()) {
                        return true;
                    } else if (g2.begin() < g1.end()) {
                        return false;
                    } else {
                        throw std::domain_error("InitialiserGenerations: generations overlap");
                    }
                });
            const size_t n_gen = _generations.size();
            std::vector<double> probs(n_gen);
            double sum_probs = 0;
            for (size_t i = 0; i < n_gen; ++i) {
                probs[i] = _generations[i].prob();
                sum_probs += probs[i];
            }
            for (size_t i = 0; i < n_gen; ++i) {
                probs[i] /= sum_probs;
            }
            _generation_distribution = std::move(DiscreteDistribution(0, std::move(probs)));
        }
    }
}

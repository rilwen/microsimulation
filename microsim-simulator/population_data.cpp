// (C) Averisera Ltd 2014-2020
#include "population_data.hpp"

namespace averisera {
    namespace microsim {
        PopulationData::PopulationData() {
        }

        PopulationData::PopulationData(PopulationData&& other)
            : persons(std::move(other.persons)) {}

        PopulationData& PopulationData::operator=(PopulationData&& other) {
            if (this != &other) {
                PopulationData copy(std::move(other));
                this->swap(copy);
            }
            return *this;
        }

        void PopulationData::swap(PopulationData& other) {
            persons.swap(other.persons);
        }
    }
}
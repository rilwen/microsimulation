#pragma once
#include "person_data.hpp"
#include <vector>

namespace averisera {
    namespace microsim {
        /** Data we construct Population from. */
        struct PopulationData {
            template <class D> using storage_t = std::vector<D>;
            storage_t<PersonData> persons;

            PopulationData(const PopulationData&) = delete;
            PopulationData& operator=(const PopulationData&) = delete;
            PopulationData();
            PopulationData(PopulationData&& other);
            PopulationData& operator=(PopulationData&& other);
            void swap(PopulationData& other);
        };

		inline void swap(PopulationData& l, PopulationData& r) {
			l.swap(r);
		}
    }
}

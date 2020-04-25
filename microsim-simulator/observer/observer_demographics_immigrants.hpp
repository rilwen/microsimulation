#pragma once
/*
(C) Averisera Ltd 2017
*/
#include "../observer.hpp"
#include "observer_demographics.hpp"

namespace averisera {
	namespace microsim {
        /** Observes immigrant population */
        class ObserverDemographicsImmigrants: public ObserverDemographics {
        public:
            /**
              @param result_saver see Observer
              @param age_ranges Vector of disjoint age ranges to observe
			@param nbr_dates Number of consecutive simulation dates under observation 
			@param own_filename_stub Stub for own output filenames. If not empty, save_results will write to new files instead of the provided stream. 
			@throw std::domain_error If age_ranges are not all disjoint
			*/
            ObserverDemographicsImmigrants(std::shared_ptr<ObserverResultSaver> result_saver, const age_ranges_type& age_ranges, size_t nbr_dates, const std::string& own_filename_stub = std::string())
                : ObserverDemographics(result_saver, "immigrants", age_ranges, nbr_dates, own_filename_stub) {}
            
            void observe(const Population& population, const Contexts& ctx) override;
        };
    }
}

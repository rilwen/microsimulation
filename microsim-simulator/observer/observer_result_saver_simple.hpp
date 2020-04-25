#pragma once
/*
(C) Averisera Ltd 2017
*/
#include "../observer_result_saver.hpp"
#include "core/dates.hpp"
#include <string>
#include <unordered_set>


namespace averisera {
    namespace microsim {
        /** Simple implementation of ObserverResultSaver. If more than one Observer shares the same saver, the results will be appended not overwritten between observers, but overwritten at each date.
         */
        class ObserverResultSaverSimple: public ObserverResultSaver {
        public:
            /**
              @param intermediate_filename Filename to use for intermediate results. Do not save them if empty.
              @param final_filename Filename to use for final results. Do not save them if empty.
             */
            ObserverResultSaverSimple(const std::string& intermediate_filename, const std::string& final_filename);

			/** Use the same filename for intermediate and final results. */
			ObserverResultSaverSimple(const std::string& filename)
				: ObserverResultSaverSimple(filename, filename) {}

            void save_intermediate(const Observer& observer, const ImmutableContext& imm_ctx, Date asof) override;

            void save_final(const Observer& observer, const ImmutableContext& imm_ctx) override;
        private:
            std::string intermediate_filename_;
            std::string final_filename_;
            std::unordered_set<Date> dates_seen_;
            bool final_started_;
        };
    }
}

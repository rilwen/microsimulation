/*
(C) Averisera Ltd 2017
*/
#include "observer_result_saver_simple.hpp"
#include "../observer.hpp"
#include <fstream>

namespace averisera {
    namespace microsim {
        ObserverResultSaverSimple::ObserverResultSaverSimple(const std::string& intermediate_filename, const std::string& final_filename)
            : intermediate_filename_(intermediate_filename), final_filename_(final_filename), final_started_(false) {}

        static void save(const Observer& observer, const ImmutableContext& imm_ctx, const std::string& filename, std::ios_base::openmode flag) {
            std::ofstream ofs(filename, flag);
            observer.save_results(ofs, imm_ctx);
        }
        
        void ObserverResultSaverSimple::save_intermediate(const Observer& observer, const ImmutableContext& imm_ctx, Date asof) {
            if (!intermediate_filename_.empty()) {
                const auto flag = (dates_seen_.find(asof) != dates_seen_.end()) ? std::ios_base::app : std::ios_base::out;
                save(observer, imm_ctx, intermediate_filename_, flag);
                dates_seen_.insert(asof);
            }
        }
        
        void ObserverResultSaverSimple::save_final(const Observer& observer, const ImmutableContext& imm_ctx) {
            if (!final_filename_.empty()) {
                save(observer, imm_ctx, final_filename_, final_started_ ? std::ios_base::app : std::ios_base::out);
                final_started_ = true;
            }
        }

    }
}

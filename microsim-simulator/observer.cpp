// (C) Averisera Ltd 2014-2020
#include "observer.hpp"
#include "observer_result_saver.hpp"
#include "core/dates.hpp"

namespace averisera {
    namespace microsim {
        Observer::Observer(std::shared_ptr<ObserverResultSaver> result_saver)
            : result_saver_(result_saver) {}
        
        Observer::~Observer() {
        }

        void Observer::save_intermediate_results(const ImmutableContext& im_ctx, Date asof) const {
            if (result_saver_) {
                result_saver_->save_intermediate(*this, im_ctx, asof);
            }
        }

        void Observer::save_final_results(const ImmutableContext& im_ctx) const {
            if (result_saver_) {
                result_saver_->save_final(*this, im_ctx);
            }
        }
    }
}

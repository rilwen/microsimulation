// (C) Averisera Ltd 2014-2020
#include "pred_alive.hpp"
#include "pred_or.hpp"
#include "../person.hpp"
#include "../contexts.hpp"

namespace averisera {
    namespace microsim {
        bool PredAlive::select(const Person& obj, const Contexts& contexts) const {
            const Date asof = contexts.asof();
            return obj.is_alive(asof);
        }

    }
}

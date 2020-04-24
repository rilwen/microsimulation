#include "operator.hpp"

namespace averisera {
    namespace microsim {
        const Feature& INSTANTANEOUS() {
            static const Feature f("__INSTANTANEOUS__");
            return f;
        }
    }
}

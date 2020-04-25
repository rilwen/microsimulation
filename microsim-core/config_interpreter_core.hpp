/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_CONFIG_INTERPETER_CORE_H
#define __AVERISERA_MS_CONFIG_INTERPETER_CORE_H

namespace averisera {
    namespace microsim {
        class ConfigInterpreter;
        
        /** @brief Core functions for the config interpreter */
        namespace ConfigInterpreterCore {
            /** Import basic functions to interpreter */
            void import(ConfigInterpreter& interpreter);
        }
    }
}

#endif // __AVERISERA_MS_CONFIG_INTERPETER_CORE_H
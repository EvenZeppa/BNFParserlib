#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>

// Définir DEBUG_ACTIVE = 1 pour activer les messages, 0 pour désactiver
#ifndef DEBUG_ACTIVE
#define DEBUG_ACTIVE 0
#endif

// Macro de debug
#define DEBUG_MSG(msg) \
    do { \
        if (DEBUG_ACTIVE) { \
            std::cerr << "[DEBUG] " << msg << std::endl; \
        } \
    } while(0)

#endif

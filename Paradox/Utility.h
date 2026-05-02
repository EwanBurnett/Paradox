#ifndef __UTILITY_H
#define __UTILITY_H

#include <stdexcept>
#include <cstdint> 

namespace Paradox {
    typedef uint32_t ParadoxError;

    [[noreturn]]
    inline void Unreachable() {
#if defined(_MSC_VER) && !defined(__clang__)
        __assume(false);
#else
        __builtin_unreachable();
#endif
    }


}

#endif// __UTILITY_H
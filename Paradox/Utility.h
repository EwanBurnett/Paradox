#ifndef __UTILITY_H
#define __UTILITY_H

#include <stdexcept>
#include <cstdint> 

#include <unordered_map>

namespace Paradox {
    enum class ParadoxError : int32_t {
        Success = 1,
        Failed = 0,

        InitializationFailed = -1,
        NotImplemented = -2,
        Timeout = -3,

        //...

        ParadoxError_MAX = INT32_MAX
    };


    ParadoxError CheckError(const ParadoxError err);

    std::string GetErrorString(const ParadoxError err); 


#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

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
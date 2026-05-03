#ifndef __LOGGER_H
#define __LOGGER_H

#include "Utility.h"
#include <cstdint> 
#include <cstdarg> 
#include <cstdio> 

//See https://en.cppreference.com/cpp/preprocessor/replace
#ifdef _MSC_VER
#define PARADOX_ERROR(message, ...) Paradox::Log::Error(__FILE__, __LINE__, __PRETTY_FUNCTION__, message, ##__VA_ARGS__)
#else
#define PARADOX_ERROR(message, ...) Paradox::Log::Error(__FILE__, __LINE__, __PRETTY_FUNCTION__, message __VA_OPT__(,) __VA_ARGS__)
#endif

namespace Paradox {
    
    enum class ELogColour : uint8_t {
        Black,
        Blue,
        Green,
        Cyan,
        Red,
        Magenta,
        Brown,
        LightGray,
        DarkGray,
        LightBlue,
        LightGreen,
        LightCyan,
        LightRed,
        LightMagenta,
        Yellow,
        White,
        ELogColour_MAX
    };
    
    class Log {
    public:
        static void Print(const ELogColour colour, const char* fmt, ...); 
        static void Message(const char* fmt, ...); 
        static void Debug(const char* fmt, ...); 
        static void Warning(const char* fmt, ...); 
        static void Error(const char* file, const size_t line, const char* function, const char* fmt, ...); 

    private: 
        static void SetConsoleColour(const ELogColour colour);
        static void Output(const char* fmt, va_list args); 
        static void Output(const char* fmt, ...);
    };

}

#endif//__LOGGER_H
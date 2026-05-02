#ifndef __LOGGER_H
#define __LOGGER_H

#include <cstdint> 
#include <cstdarg> 
#include <cstdio> 

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

    private: 
        static void SetConsoleColour(const ELogColour colour);
        static void Output(const char* fmt, va_list args); 
        static void Output(const char* fmt, ...);
    };

}

#endif//__LOGGER_H
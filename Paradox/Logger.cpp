#include "Logger.h"
#include <map> 
#include <assert.h>
#include <cstring>
#include "Profiler.h"

/**
 * @brief Maps Log Colours to their ANSI 8-Bit Foreground Console Text attributes.
 */
const std::map<Paradox::ELogColour, int32_t> kColourMappings{
    {Paradox::ELogColour::Black, 16},
    {Paradox::ELogColour::Blue, 27},
    {Paradox::ELogColour::Green, 40},
    {Paradox::ELogColour::Cyan, 117},
    {Paradox::ELogColour::Red, 196},
    {Paradox::ELogColour::Magenta, 161},
    {Paradox::ELogColour::Brown, 130},
    {Paradox::ELogColour::LightGray, 253},
    {Paradox::ELogColour::DarkGray, 244},
    {Paradox::ELogColour::LightBlue, 14},
    {Paradox::ELogColour::LightGreen, 84},
    {Paradox::ELogColour::LightCyan, 123},
    {Paradox::ELogColour::LightRed, 204},
    {Paradox::ELogColour::LightMagenta, 212},
    {Paradox::ELogColour::Yellow, 226},
    {Paradox::ELogColour::White, 255},
};

void Paradox::Log::Print(const ELogColour colour, const char* fmt, ...)
{
    SystemZoneScoped;
    //Update the console colour. 
    SetConsoleColour(colour);

    va_list args;
    va_start(args, fmt);
    Output(fmt, args);
    va_end(args);

    //Reset the console colour. 
    SetConsoleColour(ELogColour::White);
}

void Paradox::Log::Message(const char* fmt, ...)
{
    SystemZoneScoped;
    //Update the console colour. 
    SetConsoleColour(ELogColour::LightBlue);

    va_list args;
    va_start(args, fmt);
    Output(fmt, args);
    va_end(args);

    //Reset the console colour. 
    SetConsoleColour(ELogColour::White);
}

void Paradox::Log::Debug(const char* fmt, ...)
{
    SystemZoneScoped;
#if DEBUG | _DEBUG
    //Update the console colour. 
    SetConsoleColour(ELogColour::LightGreen);

    Output("[Debug]\t");
    va_list args;
    va_start(args, fmt);
    Output(fmt, args);
    va_end(args);

    //Reset the console colour. 
    SetConsoleColour(ELogColour::White);
#endif
}

void Paradox::Log::Warning(const char* fmt, ...)
{
    SystemZoneScoped;
    //Update the console colour. 
    SetConsoleColour(ELogColour::Yellow);

    Output("[Warning]\t");
    va_list args;
    va_start(args, fmt);
    Output(fmt, args);
    va_end(args);

    //Reset the console colour. 
    SetConsoleColour(ELogColour::White);
}

void Paradox::Log::Error(const char* file, const size_t line, const char* function, const char* fmt, ...)
{
    SystemZoneScoped;
    //Update the console colour. 
    SetConsoleColour(ELogColour::LightRed);

    Output("[Error]\t");
    va_list args;
    va_start(args, fmt);
    Output(fmt, args);
    va_end(args);
    Output("File: %s\nLine: %d\nFunction: %s\n", file, line, function);

    //Reset the console colour. 
    SetConsoleColour(ELogColour::White);

    assert(false && fmt);
}

void Paradox::Log::SetConsoleColour(const ELogColour colour)
{
    SystemZoneScoped;
    //Trap invalid colours. 
    if (colour >= ELogColour::ELogColour_MAX) {
        Unreachable();
        return;
    }

    //Output the 8-bit colour. 
    {
        char buf[0xf]; 
        snprintf(buf, 0xf, "\033[38;5;%dm", kColourMappings.at(colour));
        printf("%s", buf);
    }
}

void Paradox::Log::Output(const char* fmt, va_list args)
{
    SystemZoneScoped;
    if (fmt == nullptr) {
        return;
    }
    assert(strlen(fmt) > 0);
    vprintf(fmt, args);
}

void Paradox::Log::Output(const char* fmt, ...)
{
    SystemZoneScoped;
    va_list args;
    va_start(args, fmt);
    Output(fmt, args);
    va_end(args);
}

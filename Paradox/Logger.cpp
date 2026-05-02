#include "Logger.h"
#include "Utility.h"
#include <map> 
#include <assert.h>

#if WIN32
#include <Windows.h>
/**
 * @brief Maps Log Colours to their Windows Foreground Console Text attributes.
 */
const std::map<Paradox::ELogColour, int32_t> kColourMappings{
    {Paradox::ELogColour::Black, 0x0},
    {Paradox::ELogColour::Blue, 0x1},
    {Paradox::ELogColour::Green, 0x2},
    {Paradox::ELogColour::Cyan, 0x3},
    {Paradox::ELogColour::Red, 0x4},
    {Paradox::ELogColour::Magenta, 0x5},
    {Paradox::ELogColour::Brown, 0x6},
    {Paradox::ELogColour::LightGray, 0x7},
    {Paradox::ELogColour::DarkGray, 0x8},
    {Paradox::ELogColour::LightBlue, 0x9},
    {Paradox::ELogColour::LightGreen, 0xa},
    {Paradox::ELogColour::LightCyan, 0xb},
    {Paradox::ELogColour::LightRed, 0xc},
    {Paradox::ELogColour::LightMagenta, 0xd},
    {Paradox::ELogColour::Yellow, 0xe},
    {Paradox::ELogColour::White, 0xf},
};

#else 
/**
 * @brief Maps Log Colours to their Unix Foreground Console Text attributes.
 */
const std::map<Paradox::ELogColour, int32_t> kColourMappings{
    {Paradox::ELogColour::Black, 30},
    {Paradox::ELogColour::Blue, 34},
    {Paradox::ELogColour::Green, 32},
    {Paradox::ELogColour::Cyan, 36},
    {Paradox::ELogColour::Red, 32},
    {Paradox::ELogColour::Magenta, 35},
    {Paradox::ELogColour::Brown, 33},
    {Paradox::ELogColour::LightGray, 37},
    {Paradox::ELogColour::DarkGray, 37},
    {Paradox::ELogColour::LightBlue, 34},
    {Paradox::ELogColour::LightGreen, 32},
    {Paradox::ELogColour::LightCyan, 36},
    {Paradox::ELogColour::LightRed, 31},
    {Paradox::ELogColour::LightMagenta, 35},
    {Paradox::ELogColour::Yellow, 33},
    {Paradox::ELogColour::White, 37},
};

#endif

void Paradox::Log::Print(const ELogColour colour, const char* fmt, ...)
{
    //Update the console colour. 
    SetConsoleColour(colour); 

    va_list args; 
    va_start(args, fmt); 
    Output(fmt, args); 
    va_end(args); 

    //Reset the console colour. 
    SetConsoleColour(ELogColour::White);
}

void Paradox::Log::SetConsoleColour(const ELogColour colour)
{
    //Trap invalid colours. 
    if (colour >= ELogColour::ELogColour_MAX) {
        Unreachable(); 
        return; 
    }
#if WIN32
    //Get a reference to the console, and set its text attribute to the foreground colour. 
    const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE); 
    const WORD textColour = kColourMappings.at(colour); 
    SetConsoleTextAttribute(console, textColour); 
#else 
    char buf[0xff]; 
    snprintf(buf, 0xff, "\033[%dm]", kColourMappings.at(colour)); 
    printf("%s", buf); 
#endif
}

void Paradox::Log::Output(const char* fmt, va_list args)
{
    if (fmt == nullptr) {
        return; 
    }
    assert(strlen(fmt) > 0); 
    vprintf(fmt, args); 
}

void Paradox::Log::Output(const char* fmt, ...)
{
    va_list args; 
    va_start(args, fmt); 
    Output(fmt, args); 
    va_end(args); 
}

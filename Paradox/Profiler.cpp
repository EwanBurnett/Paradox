#include "Profiler.h"

void Paradox::Profiler::Init()
{
    ParadoxZoneScoped; 
    return;
}

void Paradox::Profiler::Shutdown()
{
    ParadoxZoneScoped; 
    return;
}


void Paradox::Profiler::EndFrame()
{
    ParadoxZoneScoped; 
    FrameMark;
    return;
}



const bool Paradox::Profiler::IsEnabled()
{
    ParadoxZoneScoped; 
#if defined(TRACY_ENABLE)
    return true; 
#else
    return false;
#endif
}

const bool Paradox::Profiler::IsConnected()
{
    ParadoxZoneScoped; 
    return TracyIsConnected; 
}

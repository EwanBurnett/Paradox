#ifndef __PROFILER_H
#define __PROFILER_H

#include <tracy/Tracy.hpp>
//#include <tracy/TracyVulkan.hpp>
#include <cstdint>

#define ParadoxZoneScoped ZoneScopedC(0x00a4b4); 
#define VulkanZoneScoped ZoneScopedC(0xa41e22); 
#define SystemZoneScoped ZoneScopedC(0xff6219); 
#define ResourceZoneScoped ZoneScopedC(0xa7ffc2); 


namespace Paradox {

    class Profiler {
    public: 
        static void Init(); 
        static void Shutdown(); 

        static void EndFrame(); 

        static const bool IsEnabled(); 
        static const bool IsConnected(); 
    };

}

#endif// __PROFILER_H
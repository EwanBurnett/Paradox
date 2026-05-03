#include <cstdio> 
#include <vulkan/vulkan.h>

#include "Paradox/Gpu.h"
#include "Paradox/Logger.h"

int main() {
    /*
    */
    for (size_t i = 0; i < (size_t)Paradox::ELogColour::ELogColour_MAX; ++i) {
        Paradox::Log::Print((Paradox::ELogColour)i, "Hello, RT!\t[%d]\n", i);
    }

    Paradox::Log::Warning("A warning!\n"); 
    //Paradox::Log::Error(__FILE__, __LINE__, __PRETTY_FUNCTION__, "Something went wrong!\n"); 
    PARADOX_ERROR("Uh Oh! (%d)\n", 10); 

    Paradox::GpuInitInfo initInfo = {}; 
    initInfo.applicationName = "Paradox"; 
    initInfo.applicationVersion = PackVersion(1, 0, 0); 
    initInfo.createDebug = true;

    Paradox::ParadoxError err; 
    
    Paradox::Gpu gpu; 
    err = gpu.Init(&initInfo); 

    Paradox::CheckError(err); 
    //Paradox::Log::Print(Paradox::ELogColour::Blue, Paradox::GetErrorString(err).c_str()); 


    gpu.Shutdown(); 

    return 0;     
}
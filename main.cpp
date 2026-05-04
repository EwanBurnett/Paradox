#include <cstdio> 
#include <vulkan/vulkan.h>

#include "Paradox/Gpu.h"
#include "Paradox/Logger.h"
#include "Paradox/Profiler.h"
#include "Paradox/Window.h"


int main() {
    glfwInit();
    /*
    */
    for (size_t i = 0; i < (size_t)Paradox::ELogColour::ELogColour_MAX; ++i) {
        Paradox::Log::Print((Paradox::ELogColour)i, "Hello, RT!\t[%d]\n", i);
    }

    Paradox::Log::Warning("A warning!\n");
    //Paradox::Log::Error(__FILE__, __LINE__, __PRETTY_FUNCTION__, "Something went wrong!\n"); 


    Paradox::Window window;
    window.Create(400, 300, "Paradox");
    window.SetIcon("Resources/Icons/Paradox_Icon_64_64.png");

    const Paradox::GpuInitInfo initInfo = {
        .applicationName = "Paradox",
        .applicationVersion = PackVersion(1, 0, 0),
        .createDebug = true,
        .overridePhysicalDevice = false ,
        .physicalDeviceOverrideIdx = -1,    //Set this if you want to target a specific Device. 
    };

    Paradox::ParadoxError err;

    Paradox::Gpu gpu;
    err = gpu.Init(&initInfo);

    Paradox::CheckError(err);
    //Paradox::Log::Print(Paradox::ELogColour::Blue, Paradox::GetErrorString(err).c_str()); 

    uint64_t frameIdx = 0;
    while (window.PollEvents()) {
        ParadoxZoneScoped;
        Paradox::Log::Print(Paradox::ELogColour::Cyan, "Frame %d               \r", frameIdx++);

        Paradox::Profiler::EndFrame();
    }


    gpu.Shutdown();
    window.Destroy();

    glfwTerminate();
    return 0;
}
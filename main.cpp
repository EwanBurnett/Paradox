#include <cstdio> 
#include <vulkan/vulkan.h>

#include "Paradox/Gpu.h"
#include "Paradox/Logger.h"
#include "Paradox/Profiler.h"
#include "Paradox/Window.h"
#include "Paradox/Fence.h"

#include "Paradox/Swapchain.h"
#include "Paradox/Queue.h"


int main() {
    glfwInit();

    Paradox::Window window;
    window.Create(400, 300, "Paradox");
    window.SetIcon("Resources/Icons/Paradox_Icon_64_64.png");

    const Paradox::GpuInitInfo initInfo = {
        .applicationName = "Paradox",
        .applicationVersion = PackVersion(1, 0, 0),
        .createDebug = true,
        .overridePhysicalDevice = false ,
        .physicalDeviceOverrideIdx = (uint8_t)-1,    //Set this if you want to target a specific Device. 
    };

    Paradox::ParadoxError err;

    Paradox::Gpu gpu;
    err = gpu.Init(&initInfo);
    Paradox::CheckError(err);

    Paradox::Swapchain swapchain;
    swapchain.Create(&window, &gpu, "Swapchain");

    //Create a queue. 
    Paradox::Queue mainQueue; 
    mainQueue.Create(&gpu, Paradox::EQueueType::Graphics, "Main Queue"); 
    Paradox::Queue asyncTransferQueue; 
    asyncTransferQueue.Create(&gpu, Paradox::EQueueType::Transfer, "Async Transfer Queue"); 
    Paradox::Queue asyncComputeQueue; 
    asyncComputeQueue.Create(&gpu, Paradox::EQueueType::Compute, "Async Compute Queue"); 


    uint64_t frameIdx = 0;
    while (window.PollEvents()) {
        ParadoxZoneScoped;
        uint32_t frameInFlight = frameIdx % Paradox::kFramesInFlight; 
        uint32_t imageIdx = swapchain.AcquireNextImageIndex(&gpu, UINT64_MAX, frameInFlight);

        Paradox::Log::Print(Paradox::ELogColour::Cyan, "Frame %d               \r", frameIdx);

        //Submit some work.
        {

        }

        //Present the swapchain. 
        auto r = swapchain.Present(imageIdx, mainQueue, frameInFlight); 

        if (r == Paradox::ParadoxError::OutOfDate) {
            swapchain.Recreate(&window, &gpu, "Swapchain"); 
        }

        frameIdx++; 

        Paradox::Profiler::EndFrame();
    }

    vkDeviceWaitIdle(gpu.GetDevice());  //TODO: Gpu::Flush() & Queue::Flush(); 

    asyncComputeQueue.Destroy(&gpu);
    asyncTransferQueue.Destroy(&gpu); 
    mainQueue.Destroy(&gpu); 
    swapchain.Destroy(&gpu);

    gpu.Shutdown();
    window.Destroy();

    glfwTerminate();
    return 0;
}
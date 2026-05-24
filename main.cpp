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
        .physicalDeviceOverrideIdx = (uint8_t)-1,    //Set this if you want to target a specific Device. 
    };

    Paradox::ParadoxError err;

    Paradox::Gpu gpu;
    err = gpu.Init(&initInfo);
    Paradox::CheckError(err);

    Paradox::Swapchain swapchain;
    swapchain.Create(&window, &gpu, "Swapchain");

    //Create a queue. 
    std::vector<Paradox::Queue> graphicsQueues;
    std::vector<Paradox::Queue> computeQueues;
    std::vector<Paradox::Queue> transferQueues;

    {
        Paradox::Queue tmp;
        Paradox::Log::Message("Graphics Queues\n");
        while (tmp.Create(&gpu, Paradox::EQueueType::Graphics, std::format("Graphics Queue {0}", graphicsQueues.size())) != Paradox::ParadoxError::Failed) {
            graphicsQueues.push_back(tmp);
        }

        Paradox::Log::Message("Compute Queues\n");
        while (tmp.Create(&gpu, Paradox::EQueueType::Compute, std::format("Compute Queue {0}", computeQueues.size())) != Paradox::ParadoxError::Failed) {
            computeQueues.push_back(tmp);
        }

        Paradox::Log::Message("Transfer Queues\n");
        while (tmp.Create(&gpu, Paradox::EQueueType::Transfer, std::format("Cansfer Queue {0}", transferQueues.size())) != Paradox::ParadoxError::Failed) {
            transferQueues.push_back(tmp);
        }
    }


    Paradox::Fence fence;
    fence.Create(&gpu, 0, "Fence");
    VkFence vkFence;
    gpu.CreateFence(&vkFence, true, "DbgFence");

    VkSemaphore binary;
    gpu.CreateBinarySemaphore(&binary, "DbgBinarySemaphore");

    uint64_t frameIdx = 0;
    while (window.PollEvents()) {
        ParadoxZoneScoped;
        Paradox::Log::Print(Paradox::ELogColour::Cyan, "Frame %d               \r", frameIdx++);

        //Submit some work. 
        //queue.Submit(&gpu, nullptr);

        Paradox::Profiler::EndFrame();

        fence.Signal(&gpu, frameIdx);
        fence.Wait(&gpu, frameIdx);

        swapchain.Present(0, graphicsQueues[0], 0);
    }

    gpu.DestroySemaphore(&binary);
    gpu.DestroyFence(&vkFence);
    fence.Destroy(&gpu);

    //queue.Destroy(&gpu);
    swapchain.Destroy(&gpu);

    gpu.Shutdown();
    window.Destroy();

    glfwTerminate();
    return 0;
}
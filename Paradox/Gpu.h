#ifndef __GPU_H
#define __GPU_H

#include "Utility.h"
#include "Logger.h"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>   //VMA

#include <string> 

#define PackVersion(major,minor,patch) VK_MAKE_API_VERSION(0, major, minor, patch)


namespace Paradox {

    /**
     * @brief Optional GPU Initialization Info. 
     */
    struct GpuInitInfo {
        std::string applicationName;
        uint32_t applicationVersion;
    };

    class Gpu {
    public:
        ParadoxError Init(const GpuInitInfo* pInitInfo = nullptr);
        ParadoxError Shutdown();


    private:
        VkInstance m_Instance;
        VkPhysicalDevice m_PhysicalDevice;
        VkDevice m_Device;

        VmaAllocator m_VmaAllocator;

        VkDebugUtilsMessengerEXT m_DebugMessenger;
    };
}

#endif// __GPU_H
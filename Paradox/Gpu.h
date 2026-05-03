#ifndef __GPU_H
#define __GPU_H

#include "Utility.h"
#include "Logger.h"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>   //VMA

#include <string> 
#include <bitset>

#define PackVersion(major,minor,patch) VK_MAKE_API_VERSION(0, major, minor, patch)


namespace Paradox {

    enum class EGpuFeatureCapabilities {
        None = 0, 

        Bindless = (1 << 0), 
        Ray_Tracing_Pipeline = (1 << 1),
        Ray_Query = (1 << 2),
        Dynamic_Rendering = (1 << 3),

        EGpuFeatureCapabilities_MAX,
        EGpuFeatureCapabilities_COUNT = 6
    };

    /**
     * @brief Optional GPU Initialization Info.
     */
    struct GpuInitInfo {
        std::string applicationName;
        uint32_t applicationVersion;
        bool createDebug;
    };

    class Gpu {
    public:
        Gpu();

        ParadoxError Init(const GpuInitInfo* pInitInfo = nullptr);
        ParadoxError Shutdown();

        std::bitset<(size_t)EGpuFeatureCapabilities::EGpuFeatureCapabilities_MAX> GetCapabilities() const;
    private:
        static VkResult CheckVkResult(const VkResult res, const std::string& msg = "");

        VkResult LoadInstanceFunctions(const GpuInitInfo* pInitInfo);

        VkResult CreateInstance(const GpuInitInfo* pInitInfo);
        void DestroyInstance();

        VkResult AcquirePhyicalDevice();

        VkResult CreateDevice();
        void DestroyDevice();

        VkResult CreateVMAAllocator();
        void DestroyVMAAllocator();

        VkResult CreateDebugMessenger();
        void DestroyDebugMessenger();

        VkResult SetDebugObjectName(const uint64_t handle, const VkObjectType type, const std::string& name) const;

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);


    private:
        VkInstance m_Instance;
        VkPhysicalDevice m_PhysicalDevice;
        VkDevice m_Device;

        VmaAllocator m_VmaAllocator;
        VkAllocationCallbacks* m_pAllocationCallbacks;

        VkDebugUtilsMessengerEXT m_DebugMessenger;
        std::bitset<(size_t)EGpuFeatureCapabilities::EGpuFeatureCapabilities_MAX> m_Capabilities; 
        bool m_EnableDebugUtils; 

    private:
        static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
        static PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
        static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
    };
}

#endif// __GPU_H
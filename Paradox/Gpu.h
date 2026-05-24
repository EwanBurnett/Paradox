#ifndef __GPU_H
#define __GPU_H

#include "Utility.h"
#include "Logger.h"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>   //VMA

#include <string> 
#include <bitset>

#include "Queue.h"

#define PackVersion(major,minor,patch) VK_MAKE_API_VERSION(0, major, minor, patch)


namespace Paradox {

    class Window; 

    enum class EGpuFeatureCapabilities {
        Required = 0,

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
        bool createDebug = true;
        bool overridePhysicalDevice = false;
        uint8_t physicalDeviceOverrideIdx = -1;
    };

    class Gpu {
    public:
        Gpu();

        ParadoxError Init(const GpuInitInfo* pInitInfo = nullptr);
        ParadoxError Shutdown();

        std::bitset<(size_t)EGpuFeatureCapabilities::EGpuFeatureCapabilities_MAX> GetCapabilities() const;
        bool GetCapabilitySupport(EGpuFeatureCapabilities capability) const;

    public:
        static VkResult CheckVkResult(const VkResult res, const std::string& msg = "");

        VkDevice GetDevice() const; 
        VkPhysicalDevice GetPhysicalDevice() const; 

        ParadoxError CreateQueue(VkQueue* pOutQueue, uint32_t* pOutQueueFamilyIndex, EQueueType type, const std::string& name = "") const;
        void DestroyQueue(VkQueue* pQueue) const;

        ParadoxError CreateBinarySemaphore(VkSemaphore* pOutSemaphore, const std::string& name = "") const;
        ParadoxError CreateTimelineSemaphore(VkSemaphore* pOutSemaphore, const uint64_t initialValue, const std::string& name = "") const;
        void DestroySemaphore(VkSemaphore* pSemaphore) const;

        void SignalSemaphore(VkSemaphore* pSemaphore, const uint64_t value) const;
        ParadoxError WaitSemaphore(VkSemaphore* pSemaphore, const uint64_t value, const uint64_t timeout = UINT64_MAX) const;
        ParadoxError WaitSemaphores(VkSemaphore* pSemaphores, const uint32_t numSemaphores, const uint64_t* pValues, bool waitAll = false, const uint64_t timeout = UINT64_MAX) const;

        ParadoxError CreateFence(VkFence* pOutFence, bool createSignaled = false, const std::string& name = "") const;
        void DestroyFence(VkFence* pFence) const;

        ParadoxError CreateSurface(VkSurfaceKHR* pOutSurface, const Window* pWindow, const std::string& name = "") const;
        void DestroySurface(VkSurfaceKHR* pSurface) const; 

        ParadoxError CreateSwapchain(VkSwapchainKHR* pOutSwapchain, const VkSurfaceKHR surface, VkExtent2D extents, uint32_t * pImageCount, const VkFormat format, const VkColorSpaceKHR colourSpace, const VkPresentModeKHR presentMode, const std::string& name = "") const;
        void DestroySwapchain(VkSwapchainKHR* pSwapchain) const; 


        ParadoxError CreateImageView(VkImageView* pOutImageView, const VkImage sourceImage, VkImageViewType viewType, VkFormat format, VkImageSubresourceRange subresource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }, const std::string& name = "") const;
        void DestroyImageView(VkImageView* pImageView) const; 



    private:

        VkResult LoadInstanceFunctions(const GpuInitInfo* pInitInfo);

        VkResult CreateInstance(const GpuInitInfo* pInitInfo);
        void DestroyInstance();

        VkResult AcquirePhyicalDevice(const bool overridePhysicalDevice, const uint8_t overrideIndex);

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
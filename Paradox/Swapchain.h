#ifndef __SWAPCHAIN_H
#define __SWAPCHAIN_H

#include "Utility.h"
#include "Window.h"
#include "Gpu.h"
#include "Queue.h"
#include <array>

namespace Paradox {
    constexpr uint8_t kFramesInFlight = 3u; //TODO: 

    class Swapchain {
    public: 
        Swapchain(); 

        ParadoxError Create(const Window* pWindow, const Gpu* pGpu, const std::string& name = "");
        ParadoxError Recreate(const Window* pWindow, const Gpu* pGpu, const std::string& name = "");
        ParadoxError Destroy(const Gpu* pGpu); 

        const uint32_t AcquireNextImageIndex(const Gpu* pGpu, const uint64_t timeout, const uint32_t frameInFlight = 0u) const;
        ParadoxError Present(const uint32_t imageIndex, const Queue queue, const uint32_t frameInFlight = 0u); 

        void SetSurfaceFormat(VkSurfaceFormatKHR format); 
        void SetPresentMode(VkPresentModeKHR presentMode); 


        bool IsStale() const; 

    private:

        const VkSemaphore& GetBinarySemaphore(const uint32_t frameInFlight, const uint32_t imageIndex) const;
        const VkSemaphore& GetImageAcquiredSemaphore(const uint32_t frameInFlight = 0u) const;
        const VkFence& GetFence(const uint32_t frameInFlight = 0u) const;

    private: 
        VkSwapchainKHR m_Swapchain; 
        VkSurfaceKHR m_Surface; 
        bool m_bIsStale; 

        std::array<VkFence, kFramesInFlight> m_ImageFences;
        std::array<std::vector<VkSemaphore>, kFramesInFlight> m_BinarySemaphores;
        std::array<VkSemaphore, kFramesInFlight> m_ImageReadySemaphores;

        VkExtent2D m_Extents; 
        uint32_t m_ImageCount; 
        VkFormat m_Format; 
        VkColorSpaceKHR m_ColourSpace; 
        VkPresentModeKHR m_PresentMode; 

        std::vector<VkSurfaceFormatKHR> m_SupportedSurfaceFormats;
        std::vector<VkPresentModeKHR> m_SupportedPresentModes;

        std::vector<VkImage> m_SwapchainImages; 
        std::vector<VkImageView> m_SwapchainImageViews; 



    };
}

#endif//__SWAPCHAIN_H
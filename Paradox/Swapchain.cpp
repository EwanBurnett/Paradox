#include "Swapchain.h"
#include "Profiler.h"

#include <vulkan/vk_enum_string_helper.h> 

Paradox::Swapchain::Swapchain()
{
    ResourceZoneScoped;
    m_Swapchain = VK_NULL_HANDLE;
    m_Surface = VK_NULL_HANDLE;
    m_bIsStale = true;
    m_Extents = { 0u, 0u };
    m_ImageCount = 0u;
    m_Format = VK_FORMAT_UNDEFINED;
    m_ColourSpace = VK_COLOR_SPACE_MAX_ENUM_KHR;
    m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
}

Paradox::ParadoxError Paradox::Swapchain::Create(const Window* pWindow, const Gpu* pGpu, const std::string& name)
{
    ResourceZoneScoped;

    //Create synchronisation primitives. 
    {
        for (uint8_t i = 0; i < kFramesInFlight; ++i) {
            pGpu->CreateBinarySemaphore(&m_ImageReadySemaphores[i], std::format("{} Image Ready Semaphore {}", name, i));
            pGpu->CreateFence(&m_ImageFences[i], true, std::format("{} Fence {}", name, i));
        }
    }

    //Create the surface.
    pGpu->CreateSurface(&m_Surface, pWindow);

    m_Extents = { pWindow->GetWidth(), pWindow->GetHeight() };

    //Select an appropriate format / colour space 
    {
        uint32_t numFormats = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(pGpu->GetPhysicalDevice(), m_Surface, &numFormats, nullptr);
        assert(numFormats != 0u);
        if (numFormats != 0) {
            m_SupportedSurfaceFormats.resize(numFormats);
            vkGetPhysicalDeviceSurfaceFormatsKHR(pGpu->GetPhysicalDevice(), m_Surface, &numFormats, m_SupportedSurfaceFormats.data());

            //NOTE: We're being lazy and just selecting the first swapchain image format by default. 
            SetSurfaceFormat(m_SupportedSurfaceFormats[0]);
        }
    }

    //Get supported present modes
    {
        uint32_t numPresentModes = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(pGpu->GetPhysicalDevice(), m_Surface, &numPresentModes, nullptr);
        if (numPresentModes != 0) {
            m_SupportedPresentModes.resize(numPresentModes);
            vkGetPhysicalDeviceSurfacePresentModesKHR(pGpu->GetPhysicalDevice(), m_Surface, &numPresentModes, m_SupportedPresentModes.data());
        }

        //Prefer certain present modes by default. 
        VkPresentModeKHR selectedMode = VK_PRESENT_MODE_FIFO_KHR;   //All devices are guaranteed to support FIFO. 
        for (const auto& mode : m_SupportedPresentModes) {
            if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                selectedMode = mode;
                break;
            }
            else if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                selectedMode = mode;
                continue;
            }
            else if (mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
                selectedMode = mode;
                continue;
            }
        }
        SetPresentMode(selectedMode);
    }


    Recreate(pWindow, pGpu, name);

    return ParadoxError::Success;
}

Paradox::ParadoxError Paradox::Swapchain::Recreate(const Window* pWindow, const Gpu* pGpu, const std::string& name)
{
    ResourceZoneScoped;
    //Create the Swapchain. 
    ParadoxError res = pGpu->CreateSwapchain(&m_Swapchain, m_Surface, m_Extents, &m_ImageCount, m_Format, m_ColourSpace, m_PresentMode);
    if (res == ParadoxError::OutOfDate) {
        return res;
    }


    //Retrieve images from the swapchain. 
    {
        m_SwapchainImages.clear();

        //Destroy old image views. 
        for (auto& imageView : m_SwapchainImageViews) {
            pGpu->DestroyImageView(&imageView);
        }

        m_SwapchainImageViews.clear();

        {
            uint32_t swapchainImageCount = 0;
            vkGetSwapchainImagesKHR(pGpu->GetDevice(), m_Swapchain, &swapchainImageCount, nullptr);
            m_SwapchainImages.resize(swapchainImageCount);
            vkGetSwapchainImagesKHR(pGpu->GetDevice(), m_Swapchain, &swapchainImageCount, m_SwapchainImages.data());
            m_SwapchainImages.resize(swapchainImageCount);
            m_SwapchainImageViews.resize(swapchainImageCount);
        }

        for (int i = 0; i < m_SwapchainImages.size(); i++) {
            pGpu->CreateImageView(&m_SwapchainImageViews[i], m_SwapchainImages[i], VK_IMAGE_VIEW_TYPE_2D, m_Format);
        }

        //Create synchronisation primitives. 
        for (int i = 0; i < kFramesInFlight; ++i) {
            for (auto& semaphore : m_BinarySemaphores[i]) {
                if (semaphore != VK_NULL_HANDLE) {
                    pGpu->DestroySemaphore(&semaphore);
                }
            }
            m_BinarySemaphores[i].resize(m_SwapchainImages.size());

            for (int j = 0; j < m_SwapchainImages.size(); j++) {
                pGpu->CreateBinarySemaphore(&m_BinarySemaphores[i][j], std::format("{} Binary Semaphore {}-{}", name, i, j));
            }
        }
    }

    m_bIsStale = false;

    return ParadoxError();
}


Paradox::ParadoxError Paradox::Swapchain::Destroy(const Gpu* pGpu)
{
    ResourceZoneScoped;

    for (int i = 0; i < m_SwapchainImages.size(); i++) {
        pGpu->DestroyImageView(&m_SwapchainImageViews[i]);

        for (uint8_t j = 0; j < kFramesInFlight; ++j) {
            pGpu->DestroySemaphore(&m_BinarySemaphores[j][i]);
        }
    }

    pGpu->DestroySwapchain(&m_Swapchain);
    pGpu->DestroySurface(&m_Surface);


    for (uint8_t i = 0; i < kFramesInFlight; ++i) {
        pGpu->DestroyFence(&m_ImageFences[i]);
        pGpu->DestroySemaphore(&m_ImageReadySemaphores[i]);
    }

    return ParadoxError::Success;
}

const uint32_t Paradox::Swapchain::AcquireNextImageIndex(const Gpu* pGpu, const uint64_t timeout, const uint32_t frameInFlight) const
{
    ResourceZoneScoped;
    uint32_t imageIndex = 0u;

    vkWaitForFences(pGpu->GetDevice(), 1, &m_ImageFences[frameInFlight], VK_TRUE, timeout);
    vkResetFences(pGpu->GetDevice(), 1, &m_ImageFences[frameInFlight]);

    VkResult res = vkAcquireNextImageKHR(pGpu->GetDevice(), m_Swapchain, timeout, m_ImageReadySemaphores[frameInFlight], VK_NULL_HANDLE, &imageIndex);
    if (res != VK_ERROR_OUT_OF_DATE_KHR) {
        Gpu::CheckVkResult(res);
    }

    return imageIndex;
}

Paradox::ParadoxError Paradox::Swapchain::Present(const uint32_t imageIndex, const Queue queue, const uint32_t frameInFlight)
{
    ResourceZoneScoped;

    //Submit a dummy workload to the queue, to signal the relevant semaphores. 
    {
        //TODO: Queue::Submit(); 
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &GetImageAcquiredSemaphore(frameInFlight),
            .pWaitDstStageMask = waitStages,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &GetBinarySemaphore(frameInFlight, imageIndex),
        };
        VkResult ra = vkQueueSubmit(*(VkQueue*)&queue, 1, &submitInfo, GetFence(frameInFlight));
    }

    const VkSemaphore waitSemaphores[] = {
        m_BinarySemaphores[frameInFlight].at(imageIndex)
    };

    const VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .swapchainCount = 1,
        .pSwapchains = &m_Swapchain,
        .pImageIndices = &imageIndex,
        .pResults = nullptr,
    };

    VkResult res = vkQueuePresentKHR(queue.GetQueue(), &presentInfo);

    if (res != VK_SUCCESS) {
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
            //Log::Warning("Swapchain is Out-of-date!\n");
            m_bIsStale = true;
            return ParadoxError::OutOfDate;
        }
        else {
            Log::Warning("Failed to present Swapchain! - %s\n", string_VkResult(res));
            return ParadoxError::Failed;
        }
    }
    return ParadoxError::Success;
}

void Paradox::Swapchain::SetSurfaceFormat(VkSurfaceFormatKHR format)
{
    ResourceZoneScoped;
    m_Format = format.format;
    m_ColourSpace = format.colorSpace;
}

void Paradox::Swapchain::SetPresentMode(VkPresentModeKHR presentMode)
{
    ResourceZoneScoped;
    m_PresentMode = presentMode;
}

bool Paradox::Swapchain::IsStale() const
{
    return m_bIsStale;
}

const VkSemaphore& Paradox::Swapchain::GetBinarySemaphore(const uint32_t frameInFlight, const uint32_t imageIndex) const
{
    return m_BinarySemaphores[frameInFlight][imageIndex];
}

const VkSemaphore& Paradox::Swapchain::GetImageAcquiredSemaphore(const uint32_t frameInFlight) const
{
    return m_ImageReadySemaphores[frameInFlight];
}

const VkFence& Paradox::Swapchain::GetFence(const uint32_t frameInFlight) const
{
    return m_ImageFences[frameInFlight];
}


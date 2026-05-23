#ifndef __FENCE_H
#define __FENCE_H

#include <vulkan/vulkan.h>
#include <string>

namespace Paradox {
    class Gpu; 

    class Fence {
    public: 
        Fence(); 

        void Create(const Gpu* pGpu, const uint64_t initialValue, bool createSignaled = false, const std::string& name = "");
        void Destroy(const Gpu* pGpu);

        void Signal(const Gpu* pGpu, const uint64_t value); 
        bool Wait(const Gpu* pGpu, const uint64_t value, const uint64_t timeout = UINT64_MAX); 
        static bool WaitAll(const Gpu* pGpu, const std::vector<uint64_t>& values, const std::vector<uint64_t>& timeouts, bool waitAll = true); 

    private: 
        VkSemaphore m_Fence; 
    };
}

#endif// __FENCE_H
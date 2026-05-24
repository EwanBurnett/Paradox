#ifndef __FENCE_H
#define __FENCE_H

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Paradox {
    class Gpu;

    class Fence {
    public:
        Fence();

        void Create(const Gpu* pGpu, const uint64_t initialValue, const std::string& name = "");
        void Destroy(const Gpu* pGpu);

        void Signal(const Gpu* pGpu, const uint64_t value);
        bool Wait(const Gpu* pGpu, const uint64_t value, const uint64_t timeout = UINT64_MAX);
        static bool Wait(const Gpu* pGpu, const std::vector<Paradox::Fence>& fences, const std::vector<uint64_t>& values, const uint64_t timeout, bool waitAll = true);

    private:
        VkSemaphore m_Fence;
    };
}

#endif// __FENCE_H
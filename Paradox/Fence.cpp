#include "Fence.h"
#include "Gpu.h"
#include "Profiler.h"
#include <assert.h>

Paradox::Fence::Fence()
{
    ResourceZoneScoped; 
    m_Fence = VK_NULL_HANDLE;
}

void Paradox::Fence::Create(const Gpu* pGpu, const uint64_t initialValue, const std::string& name)
{
    ResourceZoneScoped; 
    pGpu->CreateTimelineSemaphore(&m_Fence, initialValue, name); 

}

void Paradox::Fence::Destroy(const Gpu* pGpu) {
    ResourceZoneScoped; 
    pGpu->DestroySemaphore(&m_Fence); 
}

void Paradox::Fence::Signal(const Gpu* pGpu, const uint64_t value)
{
    ResourceZoneScoped; 
    pGpu->SignalSemaphore(&m_Fence, value); 
}

bool Paradox::Fence::Wait(const Gpu* pGpu, const uint64_t value, const uint64_t timeout)
{
    ResourceZoneScoped; 
    return pGpu->WaitSemaphore(&m_Fence, value, timeout) == ParadoxError::Timeout ? false : true; 
}

bool Paradox::Fence::Wait(const Gpu* pGpu, const std::vector<Paradox::Fence>& fences, const std::vector<uint64_t>& values, const uint64_t timeout, bool waitAll)
{
    ResourceZoneScoped; 
    assert(fences.size() == values.size()); 
    if (fences.size() != values.size()) {
        PARADOX_ERROR("Mis-matched number of fences / wait values! (%d/%d)\n", fences.size(), values.size()); 
        return false; 
    }

    return pGpu->WaitSemaphores((VkSemaphore*)fences.data(), fences.size(), values.data(), waitAll, timeout) == ParadoxError::Timeout ? false : true;
}


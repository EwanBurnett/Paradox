#include "Fence.h"
#include "Gpu.h"

Paradox::Fence::Fence()
{
    m_Fence = VK_NULL_HANDLE;
}

void Paradox::Fence::Create(const Gpu* pGpu, const uint64_t initialValue, bool createSignaled, const std::string& name)
{
    pGpu->CreateTimelineSemaphore(&m_Fence, initialValue, name); 

}

void Paradox::Fence::Destroy(const Gpu* pGpu) {
    pGpu->DestroySemaphore(&m_Fence); 
}

void Paradox::Fence::Signal(const Gpu* pGpu, const uint64_t value)
{
    pGpu->SignalSemaphore(&m_Fence, value); 
}

bool Paradox::Fence::Wait(const Gpu* pGpu, const uint64_t value, const uint64_t timeout)
{
    return pGpu->WaitSemaphore(&m_Fence, value, timeout) == ParadoxError::Timeout ? false : true; 
}

bool Paradox::Fence::WaitAll(const Gpu* pGpu, const std::vector<uint64_t>& values, const std::vector<uint64_t>& timeouts, bool waitAll)
{
    return false;
}

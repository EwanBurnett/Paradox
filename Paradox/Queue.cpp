#include "Queue.h"

Paradox::Queue::Queue() {
    m_Queue = VK_NULL_HANDLE; 
    m_QueueFamilyIndex = -1; 
    m_Type = EQueueType::EQueueType_MAX; 
}

Paradox::ParadoxError Paradox::Queue::Create(const Gpu* pGpu, const EQueueType type, const std::string& name)
{
    return ParadoxError::NotImplemented;
}

void Paradox::Queue::Destroy(const Gpu* pGpu)
{
    
}

void Paradox::Queue::Submit(const Gpu* pGpu, const SubmitInfo const* pSubmitInfo) const
{
}

void Paradox::Queue::Submit(const Gpu* pGpu, const uint32_t submitCount, const SubmitInfo const** ppSubmitInfos) const
{
}

void Paradox::Queue::Submit(const Gpu* pGpu, const std::vector<const SubmitInfo const*> submitInfos) const
{
}

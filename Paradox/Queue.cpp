#include "Queue.h"
#include "Gpu.h"

Paradox::Queue::Queue() {
    m_Queue = VK_NULL_HANDLE; 
    m_QueueFamilyIndex = -1; 
    m_Type = EQueueType::EQueueType_MAX; 
}

Paradox::ParadoxError Paradox::Queue::Create(const Gpu* pGpu, const EQueueType type, const std::string& name)
{
    m_Type = type; 
    return pGpu->CreateQueue(&m_Queue, &m_QueueFamilyIndex, type, name);
}

void Paradox::Queue::Destroy(const Gpu* pGpu)
{
    pGpu->DestroyQueue(&m_Queue);
    m_QueueFamilyIndex = -1; 
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

VkQueue Paradox::Queue::GetQueue() const
{
    return m_Queue;
}

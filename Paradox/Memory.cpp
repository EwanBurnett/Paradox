#include "Memory.h"
#include "Profiler.h"
#include <new>

void* operator new(std::size_t size)
{
    SystemZoneScoped;
    //Store the size of the allocation before the allocated address. 
    size += sizeof(std::size_t);
    void* ptr = malloc(size);
    *(std::size_t*)ptr = size;

    //Track the allocation with Tracy
    TracyAlloc(ptr, size);

    //Track the allocation internally
    auto& instance = Paradox::Memory::GetInstance();
    instance.SetAllocatedBytes(instance.GetAllocatedBytes() + size);
    instance.AddAllocation();

    return ((size_t*)ptr) + 1;
}

void operator delete(void* ptr) noexcept
{
    SystemZoneScoped;
    //Trap bad allocations. 
    if (ptr == nullptr) {
        return;
    }

    //Free our whole allocation
    std::size_t size = *(((std::size_t*)ptr) - 1);
    ptr = (void*)(((std::size_t*)ptr) - 1);

    //Free the allocation via Tracy
    TracyFree(ptr);

    //Track the allocation internally
    auto& instance = Paradox::Memory::GetInstance();
    instance.SetFreedBytes(instance.GetFreedBytes() + size);
    instance.RemoveAllocation();

    //Free the allocation
    free(ptr);
}

Paradox::Memory::Memory()
{
    ParadoxZoneScoped; 

    m_Allocations = 0u; 
    m_BytesAllocated = 0u; 
    m_BytesFreed = 0u; 
}

Paradox::Memory& Paradox::Memory::GetInstance()
{
    ParadoxZoneScoped;
    static Memory instance; 
    return instance; 
}

const uint64_t Paradox::Memory::GetCurrentlyUsedBytes() const
{
    ParadoxZoneScoped;
    return m_BytesAllocated - m_BytesFreed;
}

const uint64_t Paradox::Memory::GetAllocatedBytes() const
{
    ParadoxZoneScoped;
    return m_BytesAllocated;
}

const uint64_t Paradox::Memory::GetFreedBytes() const
{
    ParadoxZoneScoped;
    return m_BytesFreed;
}

const uint64_t Paradox::Memory::GetAllocationCount() const
{
    ParadoxZoneScoped;
    return m_Allocations; 
}

void Paradox::Memory::SetAllocatedBytes(const uint64_t allocated)
{
    ParadoxZoneScoped;
    std::lock_guard<std::mutex> lock(m_Lock); 
    m_BytesAllocated = allocated; 
}

void Paradox::Memory::SetFreedBytes(const uint64_t freed)
{
    ParadoxZoneScoped;
    std::lock_guard<std::mutex> lock(m_Lock); 
    m_BytesFreed = freed; 

}

void Paradox::Memory::AddAllocation()
{
    ParadoxZoneScoped;
    std::lock_guard<std::mutex> lock(m_Lock); 
    m_Allocations++; 
}

void Paradox::Memory::RemoveAllocation()
{
    ParadoxZoneScoped;
    std::lock_guard<std::mutex> lock(m_Lock); 
    m_Allocations--;
}

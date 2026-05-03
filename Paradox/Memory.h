#ifndef __MEMORY_H
#define __MEMORY_H

#include <mutex>
#include <cstdint> 
#include <string> 

namespace Paradox {
    
    class Memory {
    public: 
        Memory(); 
        Memory(const Memory&) = delete; 
        Memory(Memory&&) = delete; 
        
        static Memory& GetInstance(); 

        const uint64_t GetCurrentlyUsedBytes() const; 
        const uint64_t GetAllocatedBytes() const; 
        const uint64_t GetFreedBytes() const; 
        const uint64_t GetAllocationCount() const; 

        void SetAllocatedBytes(const uint64_t allocated);
        void SetFreedBytes(const uint64_t freed);

        void AddAllocation();
        void RemoveAllocation();

    private: 
        std::mutex m_Lock; 
        uint64_t m_Allocations; 
        uint64_t m_BytesAllocated; 
        uint64_t m_BytesFreed; 
    };

}

#endif// __MEMORY_H
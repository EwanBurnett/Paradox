#ifndef __QUEUE_H
#define __QUEUE_H

#include <cstdint> 
#include <vulkan/vulkan.h>
#include "Utility.h"

namespace Paradox {
    class Gpu; 
    class CommandBuffer;    //TODO: 
    class Fence;    //TODO: 


    enum class EPipelineStageFlags {
        TopOfPipe = 0x00000001,
        DrawIndirect = 0x00000002,
        VertexInput = 0x00000004,
        VertexShader = 0x00000008,
        TessellationControlShader = 0x00000010,
        TessellationEvaluationShader = 0x00000020,
        GeometryShader = 0x00000040,
        FragmentShader = 0x00000080,
        EarlyFragmentTests = 0x00000100,
        LateFragmentTests = 0x00000200,
        ColorAttachmentOutput = 0x00000400,
        ComputeShader = 0x00000800,
        Transfer = 0x00001000,
        BottomOfPipe = 0x00002000,
        Host = 0x00004000,
        AllGraphics = 0x00008000,
        AllCommands = 0x00010000,
        None = 0,
        TransformFeedback = 0x01000000,
        ConditionalRendering = 0x00040000,
        AccelerationStructureBuild = 0x02000000,
        RayTracingShader = 0x00200000,
        FragmentDensityProcess = 0x00800000,
        FragmentShadingRateAttachment = 0x00400000,
        TaskShader = 0x00080000,
        MeshShader = 0x00100000,
        CommandPreprocess = 0x00020000,
        ShadingRateImage = FragmentShadingRateAttachment,
        EPipelineStageFlags_MAX = 0x7Fffffff
    };

    struct SubmitInfo {
        uint32_t commandBufferCount;
        CommandBuffer* pCommandBuffers;
        uint32_t waitFenceCount;
        uint64_t pWaitValues;
        Fence* pWaitFences;
        EPipelineStageFlags* pWaitDstStageMask;
        uint32_t signalFenceCount;
        uint64_t pSignalValues;
        Fence* pSignalFences;
    };

    enum class EQueueType {
        Graphics,
        Transfer,
        Compute,
        EQueueType_MAX,
    };


    class Queue {
    public: 
        Queue();

        ParadoxError Create(const Gpu* pGpu, const EQueueType type, const std::string& name = "");
        void Destroy(const Gpu* pGpu);

        void Submit(const Gpu* pGpu, const SubmitInfo const* pSubmitInfo) const;
        void Submit(const Gpu* pGpu, const uint32_t submitCount, const SubmitInfo const** ppSubmitInfos) const;
        void Submit(const Gpu* pGpu, const std::vector<const SubmitInfo const*> submitInfos) const;

        VkQueue GetQueue() const; 

    private:
        VkQueue m_Queue;
        uint32_t m_QueueFamilyIndex;
        EQueueType m_Type;
    };
}

#endif//__QUEUE_H
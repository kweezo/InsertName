#pragma once

#include <engine/renderer/core/Swapchain.hpp>
#include <vulkan/vulkan.h>

#include "ModelInstance.hpp"

namespace renderer {
    class i_StaticInstanceData {
    public:
        i_StaticInstanceData(ModelHandle model, ShaderHandle shader);

        void UpdateDataBuffer(uint32_t threadIndex);
        void RecordCommandBuffer(i_CommandBuffer commandBuffer);

        void AddInstance(const i_ModelInstanceHandleInternal& instance);

        [[nodiscard]] i_Semaphore GetDataUploadedSemaphore() const;
    private:

        void GetDrawableInstances();
        void UploadDataToBuffer(uint32_t threadIndex);

        ModelHandle model;
        ShaderHandle shader;

        i_DataBuffer buffer;
        i_Semaphore dataUploadedSemaphore;

        uint32_t drawCount;
        std::list<i_ModelInstanceHandleInternal> instances;
        std::vector<glm::mat4> transformInstanceData;
    };
}
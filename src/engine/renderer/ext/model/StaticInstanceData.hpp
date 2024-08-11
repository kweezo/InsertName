#pragma once

#include <engine/renderer/core/Swapchain.hpp>
#include <vulkan/vulkan.h>

#include "ModelInstance.hpp"

namespace renderer {
    class i_StaticInstanceData {
    public:
        i_StaticInstanceData(ModelHandle model, std::weak_ptr<i_Shader> shader);

        void UpdateAndRecordBuffer(const i_CommandBuffer& commandBuffer);

        void AddInstance(const i_ModelInstanceHandleInternal& instance);
    private:

        void GetDrawableInstances();
        void UploadDataToBuffer(uint32_t threadIndex);
        void RecordCommandBuffer(i_CommandBuffer commandBuffer);

        ModelHandle model;
        std::weak_ptr<i_Shader> shader;

        i_DataBuffer buffer;
        i_Semaphore dataUploadedSemaphore;

        uint32_t drawCount;
        std::list<i_ModelInstanceHandleInternal> instances;
        std::vector<glm::mat4> transformInstanceData;
    };
}
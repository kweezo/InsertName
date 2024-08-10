#pragma once

#include <engine/renderer/core/Swapchain.hpp>
#include <vulkan/vulkan.h>

#include "ModelInstance.hpp"

namespace renderer {

class i_StaticInstanceData {
public:
    i_StaticInstanceData(ModelHandle model, std::weak_ptr<i_Shader> shader);

    void Update(uint32_t threadIndex);

    void AddInstance(const i_ModelInstanceHandleInternal& instance);
    void AssignCommandBuffers(const std::array<i_CommandBuffer, MAX_FRAMES_IN_FLIGHT>& commandBuffers);
private:

    void GetDrawableInstances();
    void UploadDataToBuffer(uint32_t threadIndex);
    void RecordCommandBuffer(uint32_t threadIndex);

    ModelHandle model;
    std::weak_ptr<i_Shader> shader;

    i_DataBuffer buffer;
    i_Semaphore dataUploadedSemaphore;

    std::array<i_CommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffers;

    uint32_t drawCount;
    std::list<i_ModelInstanceHandleInternal> instances;
    std::vector<glm::mat4> transformInstanceData;
};

class i_StaticInstanceManager {
    public:
        static void Init();
        static void Update();
        static void UpdateCleanup();
        static void Cleanup();

        static void AddInstance(i_ModelInstanceHandleInternal instance);

        static VkSemaphore GetRenderFinishedSemaphore(uint32_t frameInFlight);

        static void Draw();
    };
}
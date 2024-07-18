#pragma once

#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <array>
#include <memory>

#include <boost/container/flat_map.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "Model.hpp"
#include "engine/renderer/core/DataBuffer.hpp"
#include "engine/types/Transform.hpp"
#include "engine/renderer/core/GraphicsPipeline.hpp"
#include "engine/renderer/core/CommandBuffer.hpp"
#include "engine/renderer/core/Swapchain.hpp"
#include "engine/renderer/core/Fence.hpp"
#include "engine/renderer/core/Semaphore.hpp"

namespace renderer{

class __StaticModelInstance;


typedef struct __StaticModelData{
    ModelHandle model;
    __DataBuffer instanceBuffer;

    std::array<__CommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffers = {};

    uint32_t drawCount = 0;
    std::vector<std::shared_ptr<__StaticModelInstance>> instanceList = {};
} __StaticModelData;


class __StaticModelInstance{
protected:
    static void StaticInit();
    static void StaticUpdate();
    static void StaticCleanup();

    static VkSemaphore GetStaticRenderFinishedSemaphore(uint32_t imageIndex);

    static void StaticDraw(uint32_t imageIndex, __Semaphore presentSemaphore);

    virtual bool GetShouldDraw() = 0;
    virtual glm::mat4 GetModelMatrix() = 0;

    static boost::container::flat_map<ModelHandle, __StaticModelData> staticModelInstanceMap;

    static void InitializeStaticInstanceData(__StaticModelData& instanceData, ModelHandle model);
private:
    static void RecordStaticCommandBuffer(__StaticModelData& instances, uint32_t imageIndex, uint32_t threadsIndex);
    static void UploadDataToInstanceBuffer(__StaticModelData& instances, uint32_t threadIndex);

    static void RecordCommandBuffers();
    static void UpdateCleanup();

    static std::array<boost::container::flat_map<std::shared_ptr<__Shader>, std::vector<VkCommandBuffer>>, MAX_FRAMES_IN_FLIGHT> InitializeInstanceData();
    static void HandleThreads();

    const static __VertexInputDescriptions baseStaticInstanceDescriptions;

    static std::array<__Semaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
    static std::array<boost::container::flat_map<std::shared_ptr<__Shader>, std::vector<VkCommandBuffer>>, MAX_FRAMES_IN_FLIGHT> commandBuffers;

    static std::vector<std::thread> threads;
    static uint32_t threadIndex;
    static std::mutex threadSpawnMutex;
};

}


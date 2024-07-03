#pragma once

#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <array>
#include <memory>

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

class StaticModelInstance;


typedef struct StaticModelInstanceData{
    ModelHandle model = {};
    DataBuffer instanceBuffer = {};

    bool initialized = false;
    bool dataBufferInitialized = false;

    uint32_t drawCount = 0;

    std::array<bool, MAX_FRAMES_IN_FLIGHT> threadLock;    
    std::array<CommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffer = {};
    std::vector<StaticModelInstance*> instanceList = {};
} StaticModelInstanceData;


class StaticModelInstance{
public:
    static void Update();
    static void DrawStatic(uint32_t imageIndex);

protected:
    virtual bool GetShouldDraw() = 0;
    virtual glm::mat4 GetModelMatrix() = 0;

    static std::unordered_map<ModelHandle, StaticModelInstanceData> staticModelInstanceMap;

    static void StaticInstanceCleanup();
private:
    static void RecordStaticCommandBuffer(StaticModelInstanceData& instances, uint32_t imageIndex, uint32_t threadsIndex, uint32_t threadIndexInThreads);
    static void UploadDataToInstanceBuffer(StaticModelInstanceData& instances);

    static std::unordered_map<Shader, GraphicsPipeline> staticModelPipelines;

    static void InitializeMainRenderingObjects();
    static std::array<std::unordered_map<Shader, std::vector<VkCommandBuffer>>, MAX_FRAMES_IN_FLIGHT> InitializeInstanceData();
    static void HandleThreads();

    const static BufferDescriptions baseStaticInstanceDescriptions;

    static std::array<CommandBuffer, MAX_FRAMES_IN_FLIGHT> staticInstancesCommandBuffers;
    static std::array<RenderSemaphores, MAX_FRAMES_IN_FLIGHT> staticInstancesSemaphores;

    static std::vector<std::thread> threads;
    static std::vector<std::vector<bool*>> threadQueues;
    static uint32_t nextThreadInWaitlist;

    static bool mainRenderingObjectsInitialized;
};

}


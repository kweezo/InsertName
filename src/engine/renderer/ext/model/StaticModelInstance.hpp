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

class StaticModelInstance;


typedef struct __StaticModelData{
    __Model* model = nullptr;
    __DataBuffer instanceBuffer = {};

    bool initialized = false;
    bool dataBufferInitialized = false;

    uint32_t drawCount = 0;
    std::array<__CommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffer = {};
    std::vector<StaticModelInstance*> instanceList = {};
} __StaticModelData;


class StaticModelInstance{
public:
    static void Init();
    static void Update();
    static void DrawStatic(uint32_t imageIndex);

protected:
    virtual bool GetShouldDraw() = 0;
    virtual glm::mat4 GetModelMatrix() = 0;

    static boost::container::flat_map<ModelHandle, __StaticModelData> staticModelInstanceMap;

    static void Cleanup();
private:
    static void RecordStaticCommandBuffer(__StaticModelData& instances, uint32_t imageIndex, uint32_t threadsIndex);
    static void UploadDataToInstanceBuffer(__StaticModelData& instances, uint32_t threadIndex);

    static void RecordSecondaryCommandBuffers();
    static void RecordPrimaryCommandBuffer();
    static void UpdateCleanup();

    static void PrimaryCommandBufferDrawCommands(uint32_t i);

    static std::array<std::unordered_map<__Shader*, std::vector<VkCommandBuffer>>, MAX_FRAMES_IN_FLIGHT> InitializeInstanceData();
    static void HandleThreads();

    const static __VertexInputDescriptions baseStaticInstanceDescriptions;

    static std::array<__CommandBuffer, MAX_FRAMES_IN_FLIGHT> staticInstancesCommandBuffers;
    static std::array<RenderSemaphores, MAX_FRAMES_IN_FLIGHT> staticInstancesSemaphores;
    static std::array<boost::container::flat_map<__Shader*, std::vector<VkCommandBuffer>>, MAX_FRAMES_IN_FLIGHT> secondaryBuffers;

    static std::vector<std::thread> threads;
    static uint32_t threadIndex;
    static std::mutex threadSpawnMutex;
};

}


#pragma once

#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <array>
#include <memory>
#include <tuple>

#include <boost/container/flat_map.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "Model.hpp"
#include "../camera/Camera.hpp"
#include "engine/renderer/core/DataBuffer.hpp"
#include "engine/types/Transform.hpp"
#include "engine/renderer/core/GraphicsPipeline.hpp"
#include "engine/renderer/core/CommandBuffer.hpp"
#include "engine/renderer/core/Swapchain.hpp"
#include "engine/renderer/core/Fence.hpp"
#include "engine/renderer/core/Semaphore.hpp"

namespace renderer{

class _StaticModelInstance;


typedef struct _StaticInstanceData{
    ModelHandle model;
    _DataBuffer instanceBuffer;

    _Semaphore modelInstanceDataUploadedSemaphore;

    std::array<_CommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffers = {};


    uint32_t drawCount = 0;
    std::vector<std::weak_ptr<_StaticModelInstance>> instanceList = {};
} _StaticInstanceData;


class _StaticModelInstance{
protected:
    static void StaticInit();
    static void StaticUpdate();
    static void StaticUpdateCleanup();
    static void StaticCleanup();

    static VkSemaphore GetStaticRenderFinishedSemaphore(uint32_t imageIndex);

    static void StaticDraw(_Semaphore presentSemaphore, _Fence inFlightFences);

    virtual bool GetShouldDraw() = 0;
    virtual glm::mat4 GetModelMatrix() = 0;

    static boost::container::flat_map<std::string, std::shared_ptr<_StaticInstanceData>> staticModelInstanceMap;

    static void InitializeStaticInstanceData(_StaticInstanceData& instanceData, ModelHandle model);
private:
    static void RecordStaticCommandBuffer(std::weak_ptr<_StaticInstanceData> instances, uint32_t threadsIndex);
    static void UploadDataToInstanceBuffer(std::weak_ptr<_StaticInstanceData> instances, uint32_t threadIndex);

    static void PrepareCommandBuffers();

    static std::array<boost::container::flat_map<std::string, std::vector<VkCommandBuffer>>, MAX_FRAMES_IN_FLIGHT> InitializeInstanceData();
    static void HandleThreads();

    const static __VertexInputDescriptions baseStaticInstanceDescriptions;

    static std::array<_Semaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
    static std::array<boost::container::flat_map<std::string, std::vector<std::pair<VkCommandBuffer, std::vector<_Semaphore>>>>, MAX_FRAMES_IN_FLIGHT> commandBuffers;

    static std::vector<std::thread> dataUploadThreads;
    static std::vector<std::thread> commandBufferThreads;
    static uint32_t threadIndex;
    static std::mutex threadSpawnMutex;

};

}


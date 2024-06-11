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
    ModelHandle model;
    DataBuffer instanceBuffer;

    std::vector<CommandBuffer> commandBuffer;
    std::vector<StaticModelInstance*> instanceList;
} StaticModelInstanceData;


class StaticModelInstance{
public:
    static void Update();
    static void DrawStatic(uint32_t imageIndex);

protected:

    virtual bool GetShouldDraw() = 0;
    virtual glm::mat4 GetModelMatrix() = 0;

    static void RecordStaticCommandBuffer(StaticModelInstanceData& instances, uint32_t imageIndex);

    static std::unordered_map<ShaderHandle, GraphicsPipeline> staticModelPipelines;


    static std::unordered_map<ModelHandle, StaticModelInstanceData> staticModelInstanceMap;
    const static BufferDescriptions baseStaticInstanceDescriptions;

    static std::vector<CommandBuffer> staticInstancesCommandBuffers;
    static std::vector<RenderSemaphores> staticInstancesSemaphores;

};

}


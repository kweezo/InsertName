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
#include "StaticModelInstance.hpp"
#include "engine/renderer/core/DataBuffer.hpp"
#include "engine/types/Transform.hpp"
#include "engine/renderer/core/GraphicsPipeline.hpp"
#include "engine/renderer/core/CommandBuffer.hpp"
#include "engine/renderer/core/Swapchain.hpp"
#include "engine/renderer/core/Fence.hpp"
#include "engine/renderer/core/Semaphore.hpp"


#define ModelInstanceHandle std::shared_ptr<ModelInstance> 

namespace renderer{

struct ModelInstanceCreateInfo{
    Transform transform;
    bool isDynamic;
    ModelHandle model;
};


class ModelInstance : public _StaticModelInstance{
public:
    static void __Init();
    static void __Update();
    static void __UpdateCleanup();
    static void __Cleanup();

    static ModelInstanceHandle Create(ModelInstanceCreateInfo& createInfo);

    ModelInstance& operator=(const ModelInstance& other) = delete;
    ModelInstance& operator=(ModelInstance&& other) = delete;
    ModelInstance(const ModelInstance& other) = delete;
    ModelInstance(ModelInstance&& other) = delete;


    static std::array<VkSemaphore, 2> GetRenderFinishedSemaphores(uint32_t imageIndex);

    static void __Draw(_Semaphore presentSemaphore, std::array<_Fence, 2> inFlightFences);

    bool GetShouldDraw() override;
    void SetShouldDraw(bool shouldDraw);
    glm::mat4 GetModelMatrix() override;

private:
    ModelInstance(ModelInstanceCreateInfo& createInfo);

    bool shouldDraw;
    glm::mat4 model; 
};

}
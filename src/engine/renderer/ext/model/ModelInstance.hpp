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

#define ModelInstanceHandle ModelInstanceImpl*



namespace renderer{

class ModelInstanceImpl;

class ModelInstance{
public:
    static void Update();
    static ModelInstanceHandle Create(ModelHandle model, Transform transform, bool isStatic);
    static void Free(ModelInstanceHandle handle);
    static void DrawStatic(uint32_t imageIndex);
    static void Cleanup();
};


class ModelInstanceImpl : public StaticModelInstance{
public:
    ModelInstanceImpl(ModelHandle model, Transform transform, bool isStatic);

    static void Update();

    bool GetShouldDraw() override;
    void SetShouldDraw(bool shouldDraw);
    glm::mat4 GetModelMatrix() override;

    static void Cleanup();

private:
    bool shouldDraw;
    glm::mat4 model; 
};

}
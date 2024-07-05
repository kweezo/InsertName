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

class ModelInstance : public StaticModelInstance{
public:
    ModelInstance(ModelHandle model, Transform transform, bool isStatic);

    static void __Update();
    static void __Draw(uint32_t imageIndex);
    static void __Cleanup();

    bool GetShouldDraw() override;
    void SetShouldDraw(bool shouldDraw);
    glm::mat4 GetModelMatrix() override;

private:
    bool shouldDraw;
    glm::mat4 model; 
};

}
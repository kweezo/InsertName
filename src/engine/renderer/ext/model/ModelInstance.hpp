#pragma once

#include <array>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "Model.hpp"
#include "engine/types/Transform.hpp"
#include "engine/renderer/core/Fence.hpp"
#include "engine/renderer/core/Semaphore.hpp"


#define i_ModelInstanceHandleInternal std::weak_ptr<ModelInstance>
#define ModelInstanceHandle std::shared_ptr<ModelInstance>

namespace renderer {
    struct ModelInstanceCreateInfo {
        Transform transform;
        bool isDynamic;
        ModelHandle model;
    };


    class ModelInstance {
    public:
        static ModelInstanceHandle Create(ModelInstanceCreateInfo &createInfo);

        ModelInstance(ModelInstanceCreateInfo &createInfo);

        ModelInstance &operator=(const ModelInstance &other) = delete;

        ModelInstance &operator=(ModelInstance &&other) = delete;

        ModelInstance(const ModelInstance &other) = delete;

        ModelInstance(ModelInstance &&other) = delete;

        void i_AddSelfToInstanceData(std::shared_ptr<ModelInstance> self);


        static std::array<VkSemaphore, 2> GetRenderFinishedSemaphores(uint32_t imageIndex);

        static void i_Draw(i_Semaphore presentSemaphore, std::array<i_Fence, 2> inFlightFences);

        bool GetShouldDraw();

        void SetShouldDraw(bool shouldDraw);

        ModelHandle GetModel();

        glm::mat4 GetModelMatrix();

    private:
        ModelInstanceCreateInfo createInfo;

        bool shouldDraw;
        glm::mat4 model;
    };
}

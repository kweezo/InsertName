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
        ShaderHandle shader;
    };


    class ModelInstance {
    public:
        static ModelInstanceHandle Create(ModelInstanceCreateInfo &createInfo);

        ModelInstance(const ModelInstanceCreateInfo &createInfo);

        ModelInstance &operator=(const ModelInstance &other) = delete;

        ModelInstance &operator=(ModelInstance &&other) = delete;

        ModelInstance(const ModelInstance &other) = delete;

        ModelInstance(ModelInstance &&other) = delete;

        void i_AddSelfToInstanceData(const std::shared_ptr<ModelInstance>& self) const;


        static std::array<VkSemaphore, 2> GetRenderFinishedSemaphores();

        static void i_Draw(const i_Semaphore& presentSemaphore, const std::array<i_Fence, 2>& inFlightFences);

        void SetShouldDraw(bool shouldDraw);

        [[nodiscard]] bool GetShouldDraw() const;

        [[nodiscard]] ModelHandle GetModel() const;

        [[nodiscard]]glm::mat4 GetModelMatrix() const;

    private:
        ModelInstanceCreateInfo createInfo;

        bool shouldDraw;
        glm::mat4 model{};
    };
}

#include "ModelInstance.hpp"

#include "StaticInstanceManager.hpp"

namespace renderer {
    ModelInstanceHandle ModelInstance::Create(ModelInstanceCreateInfo &createInfo) {
        std::shared_ptr<ModelInstance> shared = std::make_shared<ModelInstance>(createInfo);
        shared->i_AddSelfToInstanceData(shared);
        return shared;
    }

    ModelInstance::ModelInstance(const ModelInstanceCreateInfo &createInfo) {
        this->model = glm::mat4(1.0f);
        this->model = glm::translate(this->model, createInfo.transform.pos);
        this->model = glm::scale(this->model, createInfo.transform.scale);
        //todo, face your enemies (rotation)


        shouldDraw = true;

        this->createInfo = createInfo;
    }

    void ModelInstance::i_AddSelfToInstanceData(const std::shared_ptr<ModelInstance>& self) const {
        if (!createInfo.isDynamic) {
           i_StaticInstanceManager::AddInstance(self, createInfo.shader);
        }
    }

    void ModelInstance::i_Draw(const i_Semaphore& presentSemaphore, const std::array<i_Fence, 2>& inFlightFences) {
        i_StaticInstanceManager::Draw(presentSemaphore, inFlightFences[0]);
    }

    std::array<VkSemaphore, 2> ModelInstance::GetRenderFinishedSemaphores() {
        return {i_StaticInstanceManager::GetRenderFinishedSemaphore(), i_StaticInstanceManager::GetRenderFinishedSemaphore()};
    }

    glm::mat4 ModelInstance::GetModelMatrix() const{
        return model;
    }

    bool ModelInstance::GetShouldDraw() const {
        return shouldDraw;
    }

    ModelHandle ModelInstance::GetModel() const {
        return createInfo.model;
    }


    void ModelInstance::SetShouldDraw(const bool shouldDraw){
        this->shouldDraw = shouldDraw;
    }
}

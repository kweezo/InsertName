#include "ModelInstance.hpp"

namespace renderer {
    ModelInstanceHandle ModelInstance::Create(ModelInstanceCreateInfo &createInfo) {
        std::shared_ptr<ModelInstance> shared = std::make_shared<ModelInstance>(createInfo);
        shared->i_AddSelfToInstanceData(shared);
        return shared;
    }

    ModelInstance::ModelInstance(ModelInstanceCreateInfo &createInfo) {
        this->model = glm::mat4(1.0f);
        this->model = glm::translate(this->model, createInfo.transform.pos);
        this->model = glm::scale(this->model, createInfo.transform.scale);
        //todo, face your enemies (rotation)

        if (!createInfo.isDynamic) {
            if (staticModelInstanceMap.find(createInfo.model.lock()->GetName()) == staticModelInstanceMap.end()) {
                _StaticInstanceData instanceData{};

                InitializeStaticInstanceData(instanceData, createInfo.model);

                staticModelInstanceMap.insert({
                    createInfo.model.lock()->GetName(), std::make_shared<_StaticInstanceData>(instanceData)
                });
            } else {
            }
        }

        shouldDraw = true;

        this->createInfo = createInfo;
    }

    void ModelInstance::i_AddSelfToInstanceData(std::shared_ptr<ModelInstance> self) {
        if (!createInfo.isDynamic) {
            std::weak_ptr<_StaticInstanceData> instanceData = staticModelInstanceMap[createInfo.model.lock()->
                GetName()];
            instanceData.lock()->instanceList.push_back(self);
        }
    }

    void ModelInstance::i_Draw(i_Semaphore presentSemaphore, std::array<i_Fence, 2> inFlightFences) {
        StaticDraw(presentSemaphore, inFlightFences[0]);
    }

    std::array<VkSemaphore, 2> ModelInstance::GetRenderFinishedSemaphores(uint32_t imageIndex) {
        return {GetStaticRenderFinishedSemaphore(imageIndex), GetStaticRenderFinishedSemaphore(imageIndex)};
    }

    glm::mat4 ModelInstance::GetModelMatrix() {
        return model;
    }

    bool ModelInstance::GetShouldDraw() {
        return shouldDraw;
    }

    ModelHandle ModelInstance::GetModel() {
        return createInfo.model;
    }


    void ModelInstance::SetShouldDraw(bool shouldDraw) {
        this->shouldDraw = shouldDraw;
    }
}

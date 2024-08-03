#include "ModelInstance.hpp"

namespace renderer{

ModelInstanceHandle ModelInstance::Create(ModelInstanceCreateInfo& createInfo){
    std::shared_ptr<ModelInstance> shared;
    ModelInstance(createInfo, shared);
    return shared;
}

ModelInstance::ModelInstance(ModelInstanceCreateInfo& createInfo, std::shared_ptr<ModelInstance>& shared){
    this->model = glm::mat4(1.0f);
    this->model = glm::translate(this->model, createInfo.transform.pos);
    this->model = glm::scale(this->model, createInfo.transform.scale);
    //todo, face your enemies (rotation)

    shared = std::shared_ptr<ModelInstance>(this);

    if(!createInfo.isDynamic){
        if(staticModelInstanceMap.find(createInfo.model.lock()->GetName()) == staticModelInstanceMap.end()){
            _StaticModelData instanceData{};
            instanceData.instanceList.push_back(shared);

            InitializeStaticInstanceData(instanceData, createInfo.model);

            staticModelInstanceMap.insert({createInfo.model.lock()->GetName(), std::make_shared<_StaticModelData>(instanceData)});
        }
        else{
            std::weak_ptr<_StaticModelData> instanceData = staticModelInstanceMap[createInfo.model.lock()->GetName()];
            instanceData.lock()->instanceList.push_back(shared);
        }
    }

    shouldDraw = true;
}

void ModelInstance::__Init(){
    StaticInit();
}

void ModelInstance::__Update(){
    StaticUpdate();
}

void ModelInstance::__UpdateCleanup(){
    StaticUpdateCleanup();
}

void ModelInstance::__Draw(_Semaphore presentSemaphore, std::array<_Fence, 2> inFlightFences){
    StaticDraw(presentSemaphore, inFlightFences[0]);
}

void ModelInstance::__Cleanup(){
    StaticCleanup();
}

std::array<VkSemaphore, 2> ModelInstance::GetRenderFinishedSemaphores(uint32_t imageIndex){
    return {GetStaticRenderFinishedSemaphore(imageIndex), GetStaticRenderFinishedSemaphore(imageIndex)};
}

glm::mat4 ModelInstance::GetModelMatrix(){
    return model;
}

bool ModelInstance::GetShouldDraw(){
    return shouldDraw;
}

void ModelInstance::SetShouldDraw(bool shouldDraw){
    this->shouldDraw = shouldDraw;
}

}
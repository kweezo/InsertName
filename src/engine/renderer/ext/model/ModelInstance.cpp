#include "ModelInstance.hpp"

namespace renderer{

ModelInstanceHandle ModelInstance::Create(ModelInstanceCreateInfo& createInfo){
    return std::shared_ptr<ModelInstance>(new ModelInstance(createInfo));
}

ModelInstance::ModelInstance(ModelInstanceCreateInfo& createInfo){
    this->model = glm::mat4(1.0f);
    this->model = glm::translate(this->model, createInfo.transform.pos);
    this->model = glm::scale(this->model, createInfo.transform.scale);
    //todo, face your enemies (rotation)

    if(!createInfo.isDynamic){
        if(staticModelInstanceMap.find(createInfo.model) == staticModelInstanceMap.end()){
            __StaticModelData instanceData{};
            instanceData.instanceList.emplace_back(this);

            InitializeStaticInstanceData(instanceData, createInfo.model);

            staticModelInstanceMap[createInfo.model] = instanceData;
        }
        else{
            __StaticModelData& instanceData =  staticModelInstanceMap[createInfo.model];
            instanceData.instanceList.emplace_back(this);
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

void ModelInstance::__Draw(uint32_t imageIndex, __Semaphore presentSemaphore){
    StaticDraw(imageIndex, presentSemaphore);
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
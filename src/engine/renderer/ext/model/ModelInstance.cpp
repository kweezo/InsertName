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
        __StaticModelData& modelData = staticModelInstanceMap[createInfo.model];

        modelData.instanceList.emplace_back(this);
        modelData.model = createInfo.model;
    }

    shouldDraw = true;
}

void ModelInstance::__Cleanup(){
    __StaticModelInstance::Cleanup();
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
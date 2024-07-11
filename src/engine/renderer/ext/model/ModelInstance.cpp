#include "ModelInstance.hpp"

namespace renderer{

ModelInstance::ModelInstance(ModelInstanceCreateInfo createInfo){
    this->model = glm::mat4(1.0f);
    this->model = glm::translate(this->model, createInfo.transform.pos);
    this->model = glm::scale(this->model, createInfo.transform.scale);
    //todo, face your enemies (rotation)

    if(createInfo.isStatic){
        __StaticModelData& modelData = staticModelInstanceMap[createInfo.model];

        modelData.instanceList.push_back(this);
        modelData.model = static_cast<__Model*>(createInfo.model);
    }

    shouldDraw = true;
}

void ModelInstance::__Cleanup(){
    StaticModelInstance::Cleanup();
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
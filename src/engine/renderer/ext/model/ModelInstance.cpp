#include "ModelInstance.hpp"

namespace renderer{

ModelInstance::ModelInstance(ModelHandle model, Transform transform, bool isStatic){
    this->model = glm::mat4(1.0f);
    this->model = glm::translate(this->model, transform.pos);
    this->model = glm::scale(this->model, transform.scale);
    //todo, face your enemies (rotation)

    if(isStatic){
        staticModelInstanceMap[model].instanceList.push_back(this);
        staticModelInstanceMap[model].model = static_cast<__Model*>(model);
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
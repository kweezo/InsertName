#include "ModelInstance.hpp"

namespace renderer{

std::unordered_map<ModelHandle, DataBuffer> ModelInstanceImpl::staticModelMatrices = {};

ModelInstanceImpl::ModelInstanceImpl(ModelHandle model, Transform transform, bool isStatic){
    this->model = glm::mat4(1.0f);
    this->model = glm::translate(this->model, transform.pos);
    this->model = glm::scale(this->model, transform.scale);
    //todo, face your enemies (rotation)

    if(isStatic){
        //staticModelMatrices[model].push_back(this->model);
    }
}

void ModelInstanceImpl::UpdateStaticInstances(){

}

}
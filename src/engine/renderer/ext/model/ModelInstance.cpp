#include "ModelInstance.hpp"

namespace renderer{

ModelInstanceHandle ModelInstance::Create(ModelHandle model, Transform transform, bool isStatic){
    return new ModelInstanceImpl(model, transform, isStatic);
}

void ModelInstance::Cleanup(){
    ModelInstanceImpl::Cleanup();
}

void ModelInstance::Update(){
    ModelInstanceImpl::Update();
}

void ModelInstance::Free(ModelInstanceHandle handle){
    delete handle;
}

void ModelInstanceImpl::Update(){
//TODO: clean this shit up somehow

    StaticModelInstance::Update();
}

ModelInstanceImpl::ModelInstanceImpl(ModelHandle model, Transform transform, bool isStatic){
    this->model = glm::mat4(1.0f);
    this->model = glm::translate(this->model, transform.pos);
    this->model = glm::scale(this->model, transform.scale);
    //todo, face your enemies (rotation)

    if(isStatic){
        staticModelInstanceMap[model].instanceList.push_back(this);
        staticModelInstanceMap[model].model = model;
    }

    shouldDraw = true;
}

void ModelInstanceImpl::Cleanup(){
    staticModelPipelines.clear();
    for(auto& [modelHandle, instanceDat] : staticModelInstanceMap){
        instanceDat.instanceBuffer.~DataBuffer();
    }
    staticModelInstanceMap.clear();
}

glm::mat4 ModelInstanceImpl::GetModelMatrix(){
    return model;
}

bool ModelInstanceImpl::GetShouldDraw(){
    return shouldDraw;
}

void ModelInstanceImpl::SetShouldDraw(bool shouldDraw){
    this->shouldDraw = shouldDraw;
}

}
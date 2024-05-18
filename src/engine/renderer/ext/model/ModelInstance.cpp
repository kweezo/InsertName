#include "ModelInstance.hpp"

namespace renderer{

std::unordered_map<ModelHandle, ModelInstanceData> ModelInstanceImpl::staticModelMatrices = {};

ModelInstanceImpl::ModelInstanceImpl(ModelHandle model, Transform transform, bool isStatic){
    this->model = glm::mat4(1.0f);
    this->model = glm::translate(this->model, transform.pos);
    this->model = glm::scale(this->model, transform.scale);
    //todo, face your enemies (rotation)

    if(isStatic){
        staticModelMatrices[model].instanceList.push_back(this);
    }

    shouldDraw = true;
}

void ModelInstanceImpl::UpdateStaticInstances(){
    for(auto& [buff, instances] : staticModelMatrices){
        uint32_t drawCount;
        for(ModelInstanceHandle instance : instances.instanceList){
            drawCount += instance->GetShouldDraw();
        }

        std::vector<glm::mat4> instanceModels(drawCount);

        for(ModelInstanceHandle instance : instances.instanceList){
            if(instance->GetShouldDraw()){
                instanceModels.push_back(instance->GetModelMatrix());
            }
        }

   //     instances.instanceBuffer = DataBuffer::CreateBuffer(
    }
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
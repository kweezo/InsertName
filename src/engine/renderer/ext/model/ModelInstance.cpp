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

        BufferDescriptions bufferDescriptions{};
        bufferDescriptions.bindingDescriptions.push_back({0, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE});

        bufferDescriptions.attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0});
        bufferDescriptions.attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4)});
        bufferDescriptions.attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 2 * sizeof(glm::vec4)});
        bufferDescriptions.attributeDescriptions.push_back({4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 3 * sizeof(glm::vec4)});

        instances.instanceBuffer = DataBuffer(bufferDescriptions, instanceModels.size() * sizeof(glm::mat4), instanceModels.data(), true, DATA_BUFFER_VERTEX_BIT);
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
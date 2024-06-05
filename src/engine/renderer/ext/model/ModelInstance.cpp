#include "ModelInstance.hpp"

namespace renderer{

ModelInstanceHandle ModelInstance::Create(ModelHandle model, Transform transform, bool isStatic){
    return new ModelInstanceImpl(model, transform, isStatic);
}

void ModelInstance::Update(){
    ModelInstanceImpl::Update();
}

void ModelInstanceImpl::Update(){


    bufferDescriptions.bindingDescriptions.push_back({0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX});
    bufferDescriptions.attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0});

    bufferDescriptions.bindingDescriptions.push_back({1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE});

    bufferDescriptions.attributeDescriptions.push_back({1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0});
    bufferDescriptions.attributeDescriptions.push_back({2, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4)});
    bufferDescriptions.attributeDescriptions.push_back({3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 2 * sizeof(glm::vec4)});
    bufferDescriptions.attributeDescriptions.push_back({4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 3 * sizeof(glm::vec4)});

    std::unordered_map<ShaderHandle, std::vector<ModelHandle>> models = ModelImpl::GetModelList();

    for(auto& [shader, model] : models){
        BufferDescriptions modelDescriptions = model[0]->GetExtraDescriptions();
        modelDescriptions.attributeDescriptions.insert(modelDescriptions.attributeDescriptions.end(), bufferDescriptions.attributeDescriptions.begin(),
        bufferDescriptions.attributeDescriptions.end());
        modelDescriptions.bindingDescriptions.insert(modelDescriptions.bindingDescriptions.end(), bufferDescriptions.bindingDescriptions.begin(),
        bufferDescriptions.bindingDescriptions.end());
        pipelines[shader] = GraphicsPipeline(*shader, modelDescriptions);
    }

   // pipeline = GraphicsPipeline(ModelImpl::GetShader(), shade);    
}

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
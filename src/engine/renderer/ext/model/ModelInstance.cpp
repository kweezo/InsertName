#include "ModelInstance.hpp"

namespace renderer{

std::unordered_map<ModelHandle, ModelInstanceData> ModelInstanceImpl::staticModelMatrices = {};
std::unordered_map<ShaderHandle, GraphicsPipeline> ModelInstanceImpl::pipelines = {};
BufferDescriptions ModelInstanceImpl::bufferDescriptions = {};
CommandBuffer ModelInstanceImpl::staticInstancesCommandBuffer = {};

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

void ModelInstanceImpl::UpdateStaticInstances(){
    std::vector<std::thread> threads;

    for(auto& [buff, instances] : staticModelMatrices){
        //threads.push_back(std::thread(&RecordStaticCommandBuffer, instances));
        threads.back().detach();
    }

    for(uint32_t i = 0; i < threads.size(); i++){
        if(threads[i].joinable()){
            threads[i].join();
            threads.erase(threads.begin() + i);
            i--;
        }
    }
}


void ModelInstanceImpl::RecordStaticCommandBuffer(ModelInstanceData& instances){
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

    
        BufferDescriptions instanceDescriptions;
        instanceDescriptions.attributeDescriptions.insert(instanceDescriptions.attributeDescriptions.end(), bufferDescriptions.attributeDescriptions.begin()
        + static_cast<std::vector<double>::difference_type>(1), bufferDescriptions.attributeDescriptions.end());
        instanceDescriptions.bindingDescriptions.insert(instanceDescriptions.bindingDescriptions.end(), bufferDescriptions.bindingDescriptions.begin()
        + static_cast<std::vector<double>::difference_type>(1), bufferDescriptions.bindingDescriptions.end());

        if(pipelines.find(instances.shader) == pipelines.end()){
            pipelines[instances.shader] = GraphicsPipeline(*instances.shader, instanceDescriptions);
        }

        instances.instanceBuffer = DataBuffer(bufferDescriptions, instanceModels.size() * sizeof(glm::mat4), instanceModels.data(), true, DATA_BUFFER_VERTEX_BIT);
        instances.commandBuffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY, COMMAND_BUFFER_GRAPHICS_FLAG);

        VkCommandBufferInheritanceInfo inheritanceInfo{};
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        //inheritanceInfo.renderPass = pipelines[instances.shader].GetRenderPass();
//        inheritanceInfo.framebuffer = pipelines[instances.shader].


       // instances.commandBuffer.BeginCommandBuffer(&inheritanceInfo);
        
       // instances.model->GetExtraDrawCommands();

//        instances.commandBuffer.En}dCommandBuffer();
}

void ModelInstanceImpl::DrawStatic(){



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
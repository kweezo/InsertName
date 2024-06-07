#include "StaticModelInstance.hpp"


namespace renderer{

std::unordered_map<ModelHandle, StaticModelInstanceData> StaticModelInstance::staticModelInstanceMap = {};
std::unordered_map<ShaderHandle, GraphicsPipeline> StaticModelInstance::staticModelPipelines = {};
BufferDescriptions StaticModelInstance::bufferDescriptions = {};
std::vector<CommandBuffer> StaticModelInstance::staticInstancesCommandBuffers = {};
std::vector<RenderSemaphores> StaticModelInstance::staticInstancesSemaphores = {};



void StaticModelInstance::UpdateStaticInstances(){
        if (staticInstancesCommandBuffers.empty()){
            staticInstancesCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            staticInstancesSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
                staticInstancesCommandBuffers[i] = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, COMMAND_BUFFER_GRAPHICS_FLAG);
                staticInstancesSemaphores[i].imageAvailableSemaphore = Semaphore();
                staticInstancesSemaphores[i].renderFinishedSemaphore = Semaphore();
            }
        }
    

    std::vector<std::thread> threads;

    for(auto& [buff, instances] : staticModelInstanceMap){
        for(uint32_t i = 0; MAX_FRAMES_IN_FLIGHT; i++){
            threads.push_back(std::thread(&RecordStaticCommandBuffer, std::ref(instances), i));
            threads.back().detach();
        }
    }

    for(uint32_t i = 0; i < threads.size(); i++){
        if(threads[i].joinable()){
            threads[i].join();
            threads.erase(threads.begin() + i);
            i--;
        }
    }

    for(uint32_t i = 0; i < Swapchain::GetImageCount(); i++){
        std::unordered_map<ShaderHandle, std::vector<VkCommandBuffer>> secondaryBuffers;
        
        for(auto& [buff, instances] : staticModelInstanceMap){
            secondaryBuffers[instances.shader].push_back(instances.commandBuffer[i].GetCommandBuffer());
        }

        staticInstancesCommandBuffers[i].BeginCommandBuffer(nullptr);
        for(auto& [shader, pipeline] : staticModelPipelines){
            pipeline.BeginRenderPassAndBindPipeline(i, staticInstancesCommandBuffers[i].GetCommandBuffer());

            vkCmdExecuteCommands(staticInstancesCommandBuffers[i].GetCommandBuffer(), secondaryBuffers[shader].size(), secondaryBuffers[shader].data());

            pipeline.EndRenderPass(staticInstancesCommandBuffers[i].GetCommandBuffer());
        }

        staticInstancesCommandBuffers[i].EndCommandBuffer();
    }
}


void StaticModelInstance::RecordStaticCommandBuffer(StaticModelInstanceData& instances, uint32_t imageIndex){
    uint32_t drawCount;
    for(StaticModelInstance* instance : instances.instanceList){
        drawCount += instance->GetShouldDraw();
    }

    std::vector<glm::mat4> instanceModels(drawCount);

    for(StaticModelInstance* instance : instances.instanceList){
        if(instance->GetShouldDraw()){
            instanceModels.push_back(instance->GetModelMatrix());
        }
    }

    
    BufferDescriptions instanceDescriptions;
    instanceDescriptions.attributeDescriptions.insert(instanceDescriptions.attributeDescriptions.end(), bufferDescriptions.attributeDescriptions.begin()
    + static_cast<std::vector<double>::difference_type>(1), bufferDescriptions.attributeDescriptions.end());
    instanceDescriptions.bindingDescriptions.insert(instanceDescriptions.bindingDescriptions.end(), bufferDescriptions.bindingDescriptions.begin()
    + static_cast<std::vector<double>::difference_type>(1), bufferDescriptions.bindingDescriptions.end());

    if(staticModelPipelines.find(instances.shader) == staticModelPipelines.end()){
        staticModelPipelines[instances.shader] = GraphicsPipeline(*instances.shader, instanceDescriptions);
    }

    instances.instanceBuffer = DataBuffer(bufferDescriptions, instanceModels.size() * sizeof(glm::mat4), instanceModels.data(), true,
     DATA_BUFFER_VERTEX_BIT);

    if(instances.commandBuffer.empty()){
        for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            instances.commandBuffer[i] = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY, COMMAND_BUFFER_GRAPHICS_FLAG);
        }
    }

    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = staticModelPipelines[instances.shader].GetRenderPass();
    inheritanceInfo.framebuffer = staticModelPipelines[instances.shader].GetFramebuffer(imageIndex);

    instances.commandBuffer[imageIndex].BeginCommandBuffer(&inheritanceInfo);
        
    instances.model->GetExtraDrawCommands();
    instances.model->RecordDrawCommands(instances.commandBuffer[imageIndex], drawCount);

    instances.commandBuffer[imageIndex].EndCommandBuffer();
}

void StaticModelInstance::DrawStatic(uint32_t imageIndex){
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
}


}
#include "StaticModelInstance.hpp"


namespace renderer{

boost::container::flat_map<ModelHandle, __StaticModelData> StaticModelInstance::staticModelInstanceMap = {};
std::array<__CommandBuffer, MAX_FRAMES_IN_FLIGHT> StaticModelInstance::staticInstancesCommandBuffers = {};
std::array<RenderSemaphores, MAX_FRAMES_IN_FLIGHT> StaticModelInstance::staticInstancesSemaphores = {};
uint32_t StaticModelInstance::threadIndex = 0;
std::vector<std::thread> StaticModelInstance::threads = {};
std::array<boost::container::flat_map<__Shader*, std::vector<VkCommandBuffer>>, MAX_FRAMES_IN_FLIGHT> StaticModelInstance::secondaryBuffers = {};


const __VertexInputDescriptions StaticModelInstance::baseStaticInstanceDescriptions = {{
    {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
    {1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
    {2, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4)},
    {3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 2 * sizeof(glm::vec4)},
    {4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 3 * sizeof(glm::vec4)},
    {5, 2, VK_FORMAT_R32G32_SFLOAT, 0},
    {6, 3, VK_FORMAT_R32G32B32_SFLOAT, 0},
},
{
    {0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX},
    {1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE},
    {2, sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX},
    {3, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX},
}
};

void StaticModelInstance::Init(){
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        __CommandBufferCreateInfo createInfo{};
        createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        createInfo.flags = COMMAND_BUFFER_GRAPHICS_FLAG;
        createInfo.threadIndex = 0;
        createInfo.type = __CommandBufferType::GENERIC;

        __SemaphoreCreateInfo semaphoreInfo{};

        staticInstancesCommandBuffers[i] = __CommandBuffer(createInfo);
        staticInstancesSemaphores[i].imageAvailableSemaphore = __Semaphore(semaphoreInfo);
        staticInstancesSemaphores[i].renderFinishedSemaphore = __Semaphore(semaphoreInfo);
    }
}

void StaticModelInstance::HandleThreads(){
    uint32_t threadIndex = 0;
    for(auto& [modelHandle, instances] : staticModelInstanceMap){
        threads.push_back(std::thread(&UploadDataToInstanceBuffer, std::ref(instances), threadIndex));
        threads.back().detach();

        threadIndex = (threadIndex + 1) % std::thread::hardware_concurrency();
    }

    for(uint32_t i = 0; i < threads.size(); i++){
        if(threads[i].joinable()){
            threads[i].join();
            threads.erase(threads.begin() + i);
            i--;
        }
    } 
}

void StaticModelInstance::RecordSecondaryCommandBuffers(){
    HandleThreads();
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        for(auto& [modelHandle, instanceData] : staticModelInstanceMap){
            VkCommandBuffer buf = instanceData.commandBuffer[i].GetCommandBuffer();
            secondaryBuffers[i][instanceData.model->GetShader().get()].push_back(buf);
        }
    }
}

void StaticModelInstance::RecordPrimaryCommandBuffer(){
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){

        staticInstancesCommandBuffers[i].BeginCommandBuffer(nullptr, false);
        for(auto& [shader, commandBuffers] : secondaryBuffers[i]){
            shader->GetGraphicsPipeline()->BeginRenderPassAndBindPipeline(i, staticInstancesCommandBuffers[i].GetCommandBuffer());
//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT for secondary and VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS for primary!!!
            vkCmdExecuteCommands(staticInstancesCommandBuffers[i].GetCommandBuffer(), secondaryBuffers[i][shader].size(),
             secondaryBuffers[i][shader].data());

            shader->GetGraphicsPipeline()->EndRenderPass(staticInstancesCommandBuffers[i].GetCommandBuffer());
        }

        staticInstancesCommandBuffers[i].EndCommandBuffer();
    }
}

void StaticModelInstance::UpdateCleanup(){
    for(uint32_t i = 0; i < std::thread::hardware_concurrency(); i++){
        __CommandBuffer::ResetPools(__CommandBufferType::INSTANCE, i);
    }
}

void StaticModelInstance::Update(){

    void RecordSecondaryCommandBuffers();
    void RecordPrimaryCommandBuffer();
    void UpdateCleanup();

}

void StaticModelInstance::UploadDataToInstanceBuffer(__StaticModelData& instances, uint32_t threadIndex){
    
    for(StaticModelInstance* instance : instances.instanceList){
        instances.drawCount += instance->GetShouldDraw();
    }


    std::vector<glm::mat4> instanceModels(instances.drawCount);

    uint32_t i = 0;
    for(StaticModelInstance* instance : instances.instanceList){
        if(instance->GetShouldDraw()){
            instanceModels[i] = instance->GetModelMatrix();
            i++;
        }
    }
    __DataBufferCreateInfo createInfo{};
    createInfo.data = instanceModels.data();
    createInfo.size = instanceModels.size() * sizeof(glm::mat4);
    createInfo.threadIndex = threadIndex;
    createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    createInfo.transferToLocalDeviceMemory = true;

    instances.instanceBuffer = __DataBuffer(createInfo);

    instances.dataBufferInitialized = true;     

    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){

        __CommandBufferCreateInfo createInfo{};
        createInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        createInfo.flags = COMMAND_BUFFER_GRAPHICS_FLAG;
        createInfo.threadIndex = threadIndex;
        createInfo.type = __CommandBufferType::INSTANCE;

        instances.commandBuffer[i] = __CommandBuffer(createInfo);

        threads.push_back(std::thread(&RecordStaticCommandBuffer, std::ref(instances), i, threadIndex));

        threadIndex = (threadIndex + 1) % std::thread::hardware_concurrency();
    }
}// i know this could just as well be a lambda but hell naw I aint dealing
//with the confusion it brings

void StaticModelInstance::RecordStaticCommandBuffer(__StaticModelData& instances, uint32_t imageIndex, uint32_t threadsIndex){
    __VertexInputDescriptions allDescriptions;
    std::get<0>(allDescriptions).insert(std::get<0>(allDescriptions).end(), std::get<0>(baseStaticInstanceDescriptions).begin(),
     std::get<0>(baseStaticInstanceDescriptions).end());
    std::get<1>(allDescriptions).insert(std::get<1>(allDescriptions).end(), std::get<1>(baseStaticInstanceDescriptions).begin(),
     std::get<1>(baseStaticInstanceDescriptions).end());



    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = instances.model->GetShader()->GetGraphicsPipeline()->GetRenderPass();
    inheritanceInfo.framebuffer = instances.model->GetShader()->GetGraphicsPipeline()->GetFramebuffer(imageIndex);

    instances.commandBuffer[imageIndex].BeginCommandBuffer(&inheritanceInfo, false);
        
    instances.model->GetExtraDrawCommands();
    instances.model->RecordDrawCommands(instances.commandBuffer[imageIndex], instances.drawCount);

    instances.commandBuffer[imageIndex].EndCommandBuffer();
}

void StaticModelInstance::DrawStatic(uint32_t imageIndex){
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
}

void StaticModelInstance::Cleanup(){
    for(__CommandBuffer& commandBuffer : staticInstancesCommandBuffers){
        commandBuffer.~__CommandBuffer();
    }

    for(RenderSemaphores& semaphores : staticInstancesSemaphores){
        semaphores.imageAvailableSemaphore.~__Semaphore();
        semaphores.renderFinishedSemaphore.~__Semaphore();
    }

    for(auto& [modelHandle, instanceDat] : staticModelInstanceMap){
        instanceDat.instanceBuffer.~__DataBuffer();
    }

    staticModelInstanceMap.clear();
}


}
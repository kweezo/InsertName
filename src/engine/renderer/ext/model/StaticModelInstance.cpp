#include "StaticModelInstance.hpp"


namespace renderer{

boost::container::flat_map<ModelHandle, __StaticModelData> __StaticModelInstance::staticModelInstanceMap = {};
std::array<__Semaphore, MAX_FRAMES_IN_FLIGHT> __StaticModelInstance::renderFinishedSemaphores = {};
uint32_t __StaticModelInstance::threadIndex = 0;
std::vector<std::thread> __StaticModelInstance::threads = {};
std::array<boost::container::flat_map<std::shared_ptr<__Shader>, std::vector<VkCommandBuffer>>, MAX_FRAMES_IN_FLIGHT> __StaticModelInstance::commandBuffers = {};


const __VertexInputDescriptions __StaticModelInstance::baseStaticInstanceDescriptions = {{
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

void __StaticModelInstance::StaticInit(){
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        __SemaphoreCreateInfo semaphoreInfo{};

        renderFinishedSemaphores[i] = __Semaphore(semaphoreInfo);
    }
}
    
void __StaticModelInstance::InitializeStaticInstanceData(__StaticModelData& instanceData, ModelHandle model){
    instanceData.model = model;
}

void __StaticModelInstance::HandleThreads(){
    uint32_t threadIndex = 0;
    for(auto& [modelHandle, instances] : staticModelInstanceMap){
        threads.push_back(std::thread(&UploadDataToInstanceBuffer, std::ref(instances), threadIndex));
        threadIndex = (threadIndex + 1) % std::thread::hardware_concurrency();
    }

    for(uint32_t i = 0; i < threads.size(); i++){
        if(threads[i].joinable()){
            threads[i].join();
        }
    } 
}

void __StaticModelInstance::RecordCommandBuffers(){
    HandleThreads();
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        for(auto& [modelHandle, instanceData] : staticModelInstanceMap){
            VkCommandBuffer commandBuffer = instanceData.commandBuffers[i].GetCommandBuffer();
            commandBuffers[i][instanceData.model->GetShader()].push_back(commandBuffer);
        }
    }
}

void __StaticModelInstance::UpdateCleanup(){
    for(uint32_t i = 0; i < std::thread::hardware_concurrency(); i++){
       // __CommandBuffer::ResetPools(__CommandBufferType::INSTANCE, i);
    }
}

void __StaticModelInstance::StaticUpdate(){

    RecordCommandBuffers();
    UpdateCleanup();

}

void __StaticModelInstance::UploadDataToInstanceBuffer(__StaticModelData& instances, uint32_t threadIndex){
    
    for(std::shared_ptr<__StaticModelInstance> instance : instances.instanceList){
        instances.drawCount += instance->GetShouldDraw();
    }


    std::vector<glm::mat4> instanceModels(instances.drawCount);

    uint32_t i = 0;
    for(std::shared_ptr<__StaticModelInstance> instance : instances.instanceList){
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

    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){

        __CommandBufferCreateInfo createInfo{};
        createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        createInfo.flags = COMMAND_BUFFER_GRAPHICS_FLAG;
        createInfo.threadIndex = threadIndex;
        createInfo.type = __CommandBufferType::INSTANCE;

        instances.commandBuffers[i] = __CommandBuffer(createInfo);

        threads.push_back(std::thread(&RecordStaticCommandBuffer, std::ref(instances), i, threadIndex));

        threadIndex = (threadIndex + 1) % std::thread::hardware_concurrency();
    }
}// i know this could just as well be a lambda but hell naw I aint dealing
//with the confusion it brings

void __StaticModelInstance::RecordStaticCommandBuffer(__StaticModelData& instances, uint32_t imageIndex, uint32_t threadsIndex){
    __VertexInputDescriptions allDescriptions;
    std::get<0>(allDescriptions).insert(std::get<0>(allDescriptions).end(), std::get<0>(baseStaticInstanceDescriptions).begin(),
     std::get<0>(baseStaticInstanceDescriptions).end());
    std::get<1>(allDescriptions).insert(std::get<1>(allDescriptions).end(), std::get<1>(baseStaticInstanceDescriptions).begin(),
     std::get<1>(baseStaticInstanceDescriptions).end());



    instances.commandBuffers[imageIndex].BeginCommandBuffer(nullptr, false);
    instances.model->GetShader()->GetGraphicsPipeline()->BeginRenderPassAndBindPipeline(imageIndex, instances.commandBuffers[imageIndex].GetCommandBuffer());
        
    instances.model->GetExtraDrawCommands();
    instances.model->RecordDrawCommands(instances.commandBuffers[imageIndex], instances.drawCount);

    instances.model->GetShader()->GetGraphicsPipeline()->EndRenderPass(instances.commandBuffers[imageIndex].GetCommandBuffer());
    instances.commandBuffers[imageIndex].EndCommandBuffer();
}

void __StaticModelInstance::StaticDraw(uint32_t imageIndex, __Semaphore presentSemaphore){

    std::vector<VkCommandBuffer> toSubmitCommandBuffers;

    for(auto& [shader, commandBuffers] : commandBuffers[imageIndex]){
        std::copy(commandBuffers.begin(), commandBuffers.end(), std::back_inserter(toSubmitCommandBuffers));
    }

    std::array<VkSemaphore, 1> signalSemaphores = {renderFinishedSemaphores[imageIndex].GetSemaphore()};
    std::array<VkSemaphore, 1> waitSemaphores = {presentSemaphore.GetSemaphore()};

    std::array<VkPipelineStageFlags, waitSemaphores.size()> waitDstStageMaks = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.commandBufferCount = toSubmitCommandBuffers.size();
    submitInfo.pCommandBuffers = toSubmitCommandBuffers.data();

    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitDstStageMaks.data();

    submitInfo.signalSemaphoreCount = signalSemaphores.size();
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    if(vkQueueSubmit(__Device::GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS){
        throw std::runtime_error("Failed to submit drawing command buffers for static instances");
    }
    
}

VkSemaphore __StaticModelInstance::GetStaticRenderFinishedSemaphore(uint32_t imageIndex){
    return renderFinishedSemaphores[imageIndex].GetSemaphore();
}

void __StaticModelInstance::StaticCleanup(){
    for(__Semaphore& semaphore : renderFinishedSemaphores){
        semaphore.~__Semaphore();
    }

    for(auto& [modelHandle, instanceDat] : staticModelInstanceMap){
        instanceDat.instanceBuffer.~__DataBuffer();
    }

    staticModelInstanceMap.clear();
}

}
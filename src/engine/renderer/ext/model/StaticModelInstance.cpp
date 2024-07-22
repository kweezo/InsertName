#include "StaticModelInstance.hpp"


namespace renderer{

boost::container::flat_map<ModelHandle, _StaticModelData> _StaticModelInstance::staticModelInstanceMap = {};
std::array<_Semaphore, MAX_FRAMES_IN_FLIGHT> _StaticModelInstance::renderFinishedSemaphores = {};
uint32_t _StaticModelInstance::threadIndex = 0;
std::vector<std::thread> _StaticModelInstance::dataUploadThreads = {};
std::vector<std::thread> _StaticModelInstance::commandBufferThreads = {};
std::array<boost::container::flat_map<std::shared_ptr<_Shader>, std::vector<VkCommandBuffer>>, MAX_FRAMES_IN_FLIGHT> _StaticModelInstance::commandBuffers = {};


const __VertexInputDescriptions _StaticModelInstance::baseStaticInstanceDescriptions = {{
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

void _StaticModelInstance::StaticInit(){
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        _SemaphoreCreateInfo semaphoreInfo{};

        renderFinishedSemaphores[i] = _Semaphore(semaphoreInfo);
    }
}
    
void _StaticModelInstance::InitializeStaticInstanceData(_StaticModelData& instanceData, ModelHandle model){
    instanceData.model = model;
}

void _StaticModelInstance::HandleThreads(){
    uint32_t threadIndex = 0;
    for(auto& [modelHandle, instances] : staticModelInstanceMap){
        dataUploadThreads.push_back(std::thread(&UploadDataToInstanceBuffer, std::ref(instances), threadIndex));
        threadIndex = (threadIndex + 1) % std::thread::hardware_concurrency();
    }

    for(uint32_t i = 0; i < dataUploadThreads.size(); i++){
        if(dataUploadThreads[i].joinable()){
            dataUploadThreads[i].join();
        }
    } 
    for(uint32_t i = 0; i < commandBufferThreads.size(); i++){
        if(commandBufferThreads[i].joinable()){
            commandBufferThreads[i].join();
        }
    }
    dataUploadThreads.clear();
    commandBufferThreads.clear();
}

void _StaticModelInstance::RecordCommandBuffers(){
    HandleThreads();
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        for(auto& [modelHandle, instanceData] : staticModelInstanceMap){
            VkCommandBuffer commandBuffer = instanceData.commandBuffers[i].GetCommandBuffer();
            commandBuffers[i][instanceData.model->GetShader()].push_back(commandBuffer);
        }
    }
}

void _StaticModelInstance::StaticUpdateCleanup(){
    for(uint32_t i = 0; i < std::thread::hardware_concurrency(); i++){
        _CommandBuffer::ResetPools(_CommandBufferType::INSTANCE, i);
    }

    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        for(auto& [shader, commandBuffersPerFrame] : commandBuffers[i]){
            commandBuffersPerFrame.clear(); // TODO seems very fucking inefficient
        }
    }
}

void _StaticModelInstance::StaticUpdate(){
    RecordCommandBuffers();
}

void _StaticModelInstance::UploadDataToInstanceBuffer(_StaticModelData& instances, uint32_t threadIndex){
    
    for(std::shared_ptr<_StaticModelInstance> instance : instances.instanceList){
        instances.drawCount += instance->GetShouldDraw();
    }


    std::vector<glm::mat4> instanceModels(instances.drawCount);

    uint32_t i = 0;
    for(std::shared_ptr<_StaticModelInstance> instance : instances.instanceList){
        if(instance->GetShouldDraw()){
            instanceModels[i] = instance->GetModelMatrix();
            i++;
        }
    }
    _DataBufferCreateInfo createInfo{};
    createInfo.data = instanceModels.data();
    createInfo.size = instanceModels.size() * sizeof(glm::mat4);
    createInfo.threadIndex = threadIndex;
    createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    createInfo.transferToLocalDeviceMemory = true;

    instances.instanceBuffer = _DataBuffer(createInfo);

    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){

        _CommandBufferCreateInfo createInfo{};
        createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        createInfo.flags = COMMAND_BUFFER_GRAPHICS_FLAG;
        createInfo.threadIndex = threadIndex;
        createInfo.type = _CommandBufferType::INSTANCE;

        instances.commandBuffers[i] = _CommandBuffer(createInfo);

        commandBufferThreads.push_back(std::thread(&RecordStaticCommandBuffer, std::ref(instances), i, threadIndex));

        threadIndex = (threadIndex + 1) % std::thread::hardware_concurrency();
    }
}

void _StaticModelInstance::RecordStaticCommandBuffer(_StaticModelData& instances, uint32_t imageIndex, uint32_t threadsIndex){
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

void _StaticModelInstance::StaticDraw(uint32_t frameInFlight, _Semaphore presentSemaphore, _Fence inFlightFence){

    std::vector<VkCommandBuffer> toSubmitCommandBuffers;

    for(auto& [shader, commandBuffers] : commandBuffers[frameInFlight]){
        std::copy(commandBuffers.begin(), commandBuffers.end(), std::back_inserter(toSubmitCommandBuffers));
    }

    std::array<VkSemaphore, 1> signalSemaphores = {renderFinishedSemaphores[frameInFlight].GetSemaphore()};
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

    if(vkQueueSubmit(_Device::GetGraphicsQueue(), 1, &submitInfo, inFlightFence.GetFence()) != VK_SUCCESS){
        throw std::runtime_error("Failed to submit drawing command buffers for static instances");
    }
}

VkSemaphore _StaticModelInstance::GetStaticRenderFinishedSemaphore(uint32_t imageIndex){
    return renderFinishedSemaphores[imageIndex].GetSemaphore();
}

void _StaticModelInstance::StaticCleanup(){
    for(_Semaphore& semaphore : renderFinishedSemaphores){
        semaphore.~_Semaphore();
    }

    for(auto& [modelHandle, instanceDat] : staticModelInstanceMap){
        instanceDat.instanceBuffer.~_DataBuffer();
    }

    staticModelInstanceMap.clear();
}

}
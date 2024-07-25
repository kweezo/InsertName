#include "StaticModelInstance.hpp"


namespace renderer{

boost::container::flat_map<std::string, std::shared_ptr<_StaticModelData>> _StaticModelInstance::staticModelInstanceMap = {};
std::array<_Semaphore, MAX_FRAMES_IN_FLIGHT> _StaticModelInstance::renderFinishedSemaphores = {};
uint32_t _StaticModelInstance::threadIndex = 0;
std::vector<std::thread> _StaticModelInstance::dataUploadThreads = {};
std::vector<std::thread> _StaticModelInstance::commandBufferThreads = {};
std::array<boost::container::flat_map<std::string, std::vector<VkCommandBuffer>>, MAX_FRAMES_IN_FLIGHT> _StaticModelInstance::commandBuffers = {};


const __VertexInputDescriptions _StaticModelInstance::baseStaticInstanceDescriptions = {{
    {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
    {1, 0, VK_FORMAT_R32G32_SFLOAT, 0},
    {2, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
    {3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
    {4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4)},
    {5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 2 * sizeof(glm::vec4)},
    {6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 3 * sizeof(glm::vec4)},
},
{
    {0, sizeof(glm::vec3) * 2 + sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX},
    {1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE},
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
    for(auto& [modelName, instances] : staticModelInstanceMap){
        dataUploadThreads.push_back(std::thread(&UploadDataToInstanceBuffer, instances, threadIndex));
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

void _StaticModelInstance::PrepareCommandBuffers(){
    HandleThreads();
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        commandBuffers[i].clear();
        for(auto& [modelName, instanceData] : staticModelInstanceMap){
            VkCommandBuffer commandBuffer = instanceData->commandBuffers[i].GetCommandBuffer();
            {
                std::shared_ptr<_Shader> shader = instanceData->model.lock()->GetShader().lock();
                commandBuffers[i][shader->GetName()].push_back(commandBuffer);
            }
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
    PrepareCommandBuffers();
}

void _StaticModelInstance::UploadDataToInstanceBuffer(std::weak_ptr<_StaticModelData> instances, uint32_t threadIndex){

    std::shared_ptr<_StaticModelData> instancesShared = instances.lock();
    instancesShared->drawCount = 0;
    for(std::shared_ptr<_StaticModelInstance> instance : instancesShared->instanceList){
        instancesShared->drawCount += instance->GetShouldDraw();
    }


    std::vector<glm::mat4> instanceModels(instances.lock()->drawCount);

    uint32_t i = 0;
    for(std::shared_ptr<_StaticModelInstance> instance : instancesShared->instanceList){
        if(instance->GetShouldDraw()){
            instanceModels[i] = instance->GetModelMatrix();
            i++;
        }
    }
    _DataBufferCreateInfo dataBufferInfo{};
    dataBufferInfo.data = instanceModels.data();
    dataBufferInfo.size = instanceModels.size() * sizeof(glm::mat4);
    dataBufferInfo.threadIndex = threadIndex;
    dataBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    dataBufferInfo.transferToLocalDeviceMemory = true;

    instancesShared->instanceBuffer = _DataBuffer(dataBufferInfo);


    _CommandBufferCreateInfo commandBufferInfo{};
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.flags = COMMAND_BUFFER_GRAPHICS_FLAG;
    commandBufferInfo.threadIndex = threadIndex;
    commandBufferInfo.type = _CommandBufferType::INSTANCE;

    instancesShared->commandBuffers[_Swapchain::GetFrameInFlight()] = _CommandBuffer(commandBufferInfo);

    commandBufferThreads.push_back(std::thread(&RecordStaticCommandBuffer, instances, threadIndex));

    threadIndex = (threadIndex + 1) % std::thread::hardware_concurrency();
}

void _StaticModelInstance::RecordStaticCommandBuffer(std::weak_ptr<_StaticModelData> instances, uint32_t threadsIndex){
    __VertexInputDescriptions allDescriptions;
    std::get<0>(allDescriptions).insert(std::get<0>(allDescriptions).end(), std::get<0>(baseStaticInstanceDescriptions).begin(),
     std::get<0>(baseStaticInstanceDescriptions).end());
    std::get<1>(allDescriptions).insert(std::get<1>(allDescriptions).end(), std::get<1>(baseStaticInstanceDescriptions).begin(),
     std::get<1>(baseStaticInstanceDescriptions).end());


    {
        std::shared_ptr<_StaticModelData> instancesShared = instances.lock(); 
        std::shared_ptr<_Model> model = instancesShared->model.lock();
        std::shared_ptr<_Shader> shader = model->GetShader().lock();

        instancesShared->commandBuffers[_Swapchain::GetFrameInFlight()].BeginCommandBuffer(nullptr, false);
        shader->GetGraphicsPipeline()->BeginRenderPassAndBindPipeline(instancesShared->commandBuffers[_Swapchain::GetFrameInFlight()].GetCommandBuffer());

        std::array<VkBuffer, 1> vertexBuffers = {instancesShared->instanceBuffer.GetBuffer()};
        std::array<VkDeviceSize, 1> offsets = {0};
        std::array<VkDescriptorSet, 1> descriptorSets = {shader->GetDescriptorSet()};

        vkCmdBindVertexBuffers(instancesShared->commandBuffers[_Swapchain::GetFrameInFlight()].GetCommandBuffer(), 1, 4, vertexBuffers.data(), offsets.data());

        vkCmdBindDescriptorSets(instancesShared->commandBuffers[_Swapchain::GetFrameInFlight()].GetCommandBuffer(),
         VK_PIPELINE_BIND_POINT_GRAPHICS, shader->GetGraphicsPipeline()->GetPipelineLayout(), 0,
          descriptorSets.size(), descriptorSets.data(), 0, nullptr);

        model->GetExtraDrawCommands();
        model->RecordDrawCommands(instancesShared->commandBuffers[_Swapchain::GetFrameInFlight()], instancesShared->drawCount);

        shader->GetGraphicsPipeline()->EndRenderPass(instancesShared->commandBuffers[_Swapchain::GetFrameInFlight()].GetCommandBuffer());
        instancesShared->commandBuffers[_Swapchain::GetFrameInFlight()].EndCommandBuffer();
    }
}

void _StaticModelInstance::StaticDraw( _Semaphore presentSemaphore, _Fence inFlightFence){

    std::vector<VkCommandBuffer> toSubmitCommandBuffers;

    for(auto& [shader, commandBuffers] : commandBuffers[_Swapchain::GetFrameInFlight()]){
        std::copy(commandBuffers.begin(), commandBuffers.end(), std::back_inserter(toSubmitCommandBuffers));
    }

    std::array<VkSemaphore, 1> signalSemaphores = {renderFinishedSemaphores[_Swapchain::GetFrameInFlight()].GetSemaphore()};
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

    for(auto& [modelName, instanceDat] : staticModelInstanceMap){
        instanceDat->instanceBuffer.~_DataBuffer();
    }

    staticModelInstanceMap.clear();
}

}
#include "StaticModelInstance.hpp"


namespace renderer{

std::unordered_map<ModelHandle, StaticModelInstanceData> StaticModelInstance::staticModelInstanceMap = {};
std::unordered_map<ShaderHandle, GraphicsPipeline> StaticModelInstance::staticModelPipelines = {};
std::array<CommandBuffer, MAX_FRAMES_IN_FLIGHT> StaticModelInstance::staticInstancesCommandBuffers = {};
std::array<RenderSemaphores, MAX_FRAMES_IN_FLIGHT> StaticModelInstance::staticInstancesSemaphores = {};
bool StaticModelInstance::mainRenderingObjectsInitialized = false;


const BufferDescriptions StaticModelInstance::baseStaticInstanceDescriptions = {{
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

void StaticModelInstance::InitializeMainRenderingObjects(){
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        staticInstancesCommandBuffers[i] = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, COMMAND_BUFFER_GRAPHICS_FLAG, 0); //ensure no more concurrent access from other files
        staticInstancesSemaphores[i].imageAvailableSemaphore = Semaphore();
        staticInstancesSemaphores[i].renderFinishedSemaphore = Semaphore();
    }
    mainRenderingObjectsInitialized = true;
}

std::array<std::unordered_map<ShaderHandle, std::vector<VkCommandBuffer>>, MAX_FRAMES_IN_FLIGHT> 
StaticModelInstance::InitializeInstanceData(){
    std::array<std::unordered_map<ShaderHandle, std::vector<VkCommandBuffer>>, MAX_FRAMES_IN_FLIGHT> secondaryBuffers;
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        
        uint32_t y = 0;
        for(auto& [buff, instances] : staticModelInstanceMap){
            if(instances.initialized){
                continue;
            }
            uint32_t threadCount = std::thread::hardware_concurrency();
            for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
                instances.commandBuffer[i] = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY, COMMAND_BUFFER_GRAPHICS_FLAG, (i % threadCount) * (y + 1) + 1);
                if(i >= threadCount){
                    instances.threadLock[i] = true;
                }
                else{
                    instances.threadLock[i] = false;
                }
            }
            secondaryBuffers[y][instances.model->GetShader()].push_back(instances.commandBuffer[i].GetCommandBuffer());
            y++;

            instances.initialized = true;
        }
    }
    return secondaryBuffers;
}

void StaticModelInstance::HandleThreads(){
    uint32_t y = 0;
    std::vector<StaticModelInstanceData*> modelInstanceMapPtrs(staticModelInstanceMap.size());
    for(auto& [buff, instances] : staticModelInstanceMap){
        modelInstanceMapPtrs[y] = &instances;
        y++;
    }

    y = 0;

    std::vector<std::thread> threads;
    for(auto& [buff, instances] : staticModelInstanceMap){
        for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            threads.push_back(std::thread(&RecordStaticCommandBuffer, std::ref(instances), i, y, modelInstanceMapPtrs));
            threads.back().detach();
        }
        y++;
    }

    for(uint32_t i = 0; i < threads.size(); i++){
        if(threads[i].joinable()){
            threads[i].join();
            threads.erase(threads.begin() + i);
            i--;
        }
    } 
}

void StaticModelInstance::Update(){
    if(!mainRenderingObjectsInitialized){
        InitializeMainRenderingObjects();
    }
    
    std::array<std::unordered_map<ShaderHandle, std::vector<VkCommandBuffer>>, MAX_FRAMES_IN_FLIGHT>
    secondaryBuffers = InitializeInstanceData();
    HandleThreads();

    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){

        staticInstancesCommandBuffers[i].BeginCommandBuffer(nullptr);
        for(auto& [shader, pipeline] : staticModelPipelines){
            pipeline.BeginRenderPassAndBindPipeline(i, staticInstancesCommandBuffers[i].GetCommandBuffer());
//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT for secondary and VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS for primary!!!
            vkCmdExecuteCommands(staticInstancesCommandBuffers[i].GetCommandBuffer(), secondaryBuffers[i][shader].size(),
             secondaryBuffers[i][shader].data());

            pipeline.EndRenderPass(staticInstancesCommandBuffers[i].GetCommandBuffer());
        }

        staticInstancesCommandBuffers[i].EndCommandBuffer();
    }
}


void StaticModelInstance::RecordStaticCommandBuffer(StaticModelInstanceData& instances, uint32_t imageIndex, uint32_t instancesIndex, std::vector<StaticModelInstanceData*>
    modelInstanceMapPtrs){
    while(instances.threadLock[imageIndex]);

    uint32_t drawCount = 0;
    for(StaticModelInstance* instance : instances.instanceList){
        drawCount += instance->GetShouldDraw();
    }


    std::vector<glm::mat4> instanceModels(drawCount);

    uint32_t i = 0;
    for(StaticModelInstance* instance : instances.instanceList){
        if(instance->GetShouldDraw()){
            instanceModels[i] = instance->GetModelMatrix();
            i++;
        }
    }

    
    BufferDescriptions allDescriptions;
    allDescriptions.attributeDescriptions.insert(allDescriptions.attributeDescriptions.end(), baseStaticInstanceDescriptions.attributeDescriptions.begin(),
     baseStaticInstanceDescriptions.attributeDescriptions.end());
    allDescriptions.bindingDescriptions.insert(allDescriptions.bindingDescriptions.end(), baseStaticInstanceDescriptions.bindingDescriptions.begin(),
     baseStaticInstanceDescriptions.bindingDescriptions.end());

    if(staticModelPipelines.find(instances.model->GetShader()) == staticModelPipelines.end()){
        staticModelPipelines[instances.model->GetShader()] = GraphicsPipeline(instances.model->GetShader(), allDescriptions);
    }

 //   instances.instanceBuffer = DataBuffer(baseStaticInstanceDescriptions, instanceModels.size() * sizeof(glm::mat4), instanceModels.data(), true,
 //    DATA_BUFFER_VERTEX_BIT);



    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = staticModelPipelines[instances.model->GetShader()].GetRenderPass();
    inheritanceInfo.framebuffer = staticModelPipelines[instances.model->GetShader()].GetFramebuffer(imageIndex);

    instances.commandBuffer[imageIndex].BeginCommandBuffer(&inheritanceInfo);
        
    instances.model->GetExtraDrawCommands();
    instances.model->RecordDrawCommands(instances.commandBuffer[imageIndex], drawCount);

    instances.commandBuffer[imageIndex].EndCommandBuffer();

    uint32_t threadCount = std::thread::hardware_concurrency();
    if(instancesIndex + threadCount < modelInstanceMapPtrs.size()){
        modelInstanceMapPtrs[instancesIndex + threadCount]->threadLock[imageIndex] = false;
    }
}

void StaticModelInstance::DrawStatic(uint32_t imageIndex){
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
}

void StaticModelInstance::StaticInstanceCleanup(){
    for(CommandBuffer& commandBuffer : staticInstancesCommandBuffers){
        commandBuffer.~CommandBuffer();
    }

    for(RenderSemaphores& semaphores : staticInstancesSemaphores){
        semaphores.imageAvailableSemaphore.~Semaphore();
        semaphores.renderFinishedSemaphore.~Semaphore();
    }


    staticModelPipelines.clear();
    staticModelInstanceMap.clear();
}


}
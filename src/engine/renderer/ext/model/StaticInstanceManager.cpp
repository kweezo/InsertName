//
// Created by jakob on 8/9/24.
//

#include "StaticInstanceManager.hpp"

namespace renderer {
    void i_StaticInstanceManager::Init() {
        for (i_Semaphore &semaphore: renderFinishedSemaphores) {
            i_SemaphoreCreateInfo info{};

            semaphore = i_Semaphore(info);
        }
    }

    void i_StaticInstanceManager::Update() {
        HandleCommandBuffers();
    }

    void i_StaticInstanceManager::Cleanup() {
        for (i_Semaphore &semaphore: renderFinishedSemaphores) {
            semaphore.Destruct();
        }
    }

    void i_StaticInstanceManager::AddInstance(const i_ModelInstanceHandleInternal &instance,
                                              const std::weak_ptr<i_Shader> &shader) {
        ModelHandle model = instance.lock()->GetModel();
        std::shared_ptr<i_Shader> shared = shader.lock();

        if (instanceData.find(model) == instanceData.end()) {
            instanceData[model] = std::make_shared<i_StaticInstanceData>(model, shader);

            if(instanceDataPerShader.find(shared->GetName()) == instanceDataPerShader.end()) {

                for(i_CommandBuffer& buffer : instanceDataPerShader[shared->GetName()].first) {
                    i_CommandBufferCreateInfo info{};

                    info.threadIndex = (currThreadIndex++) % std::thread::hardware_concurrency();
                    info.type = i_CommandBufferType::INSTANCE;
                    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                    info.flags = COMMAND_BUFFER_GRAPHICS_FLAG | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;//TODO i mean it will be
                    //updated often but is it worth it? do perfomrance tests

                    buffer = i_CommandBuffer(info);
                }

            }
            instanceDataPerShader[shared->GetName()].second.push_front(instanceData[model]);
        }

        instanceData[model]->AddInstance(instance);
    }


    void i_StaticInstanceManager::HandleCommandBuffers() {
    }

    void i_StaticInstanceManager::RecordCommandBuffer(const std::weak_ptr<i_Shader>& shader,
                                                      const std::list<std::shared_ptr<i_StaticInstanceData>>& instanceData,
                                                      i_CommandBuffer commandBuffer) {
        std::shared_ptr<i_Shader> shared = shader.lock();

        commandBuffer.BeginCommandBuffer(nullptr, false);
        shared->GetGraphicsPipeline()->BeginRenderPassAndBindPipeline(commandBuffer.GetCommandBuffer());

        for(const std::shared_ptr<i_StaticInstanceData>& data : instanceData) {
            data->UpdateAndRecordBuffer(commandBuffer);
        }

        shared->GetGraphicsPipeline()->EndRenderPass(commandBuffer.GetCommandBuffer());
        commandBuffer.EndCommandBuffer();

    }


    void i_StaticInstanceManager::Draw() {

    }


    VkSemaphore i_StaticInstanceManager::GetRenderFinishedSemaphore(uint32_t frameInFlight) {
        return renderFinishedSemaphores[frameInFlight].GetSemaphore();
    }
}

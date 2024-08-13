//
// Created by jakob on 8/9/24.
//

#include "StaticInstanceManager.hpp"

namespace renderer {
    boost::container::flat_map<std::string, std::shared_ptr<i_StaticInstanceData> > i_StaticInstanceManager::instanceData = {};
    boost::container::flat_map<std::string, std::pair<std::array<i_CommandBuffer, MAX_FRAMES_IN_FLIGHT>,
        std::list<std::shared_ptr<i_StaticInstanceData> > > >
    i_StaticInstanceManager::instanceDataPerShader = {}; //shitty name but I couldnt give less than 2 shits

    uint32_t i_StaticInstanceManager::currThreadIndex = 0;

    std::array<i_Semaphore, MAX_FRAMES_IN_FLIGHT> i_StaticInstanceManager::renderFinishedSemaphores = {};

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
                                              const ShaderHandle &shader) {
        ModelHandle model = instance.lock()->GetModel();
        std::string modelName = model.lock()->GetName();
        std::shared_ptr<i_Shader> shared = shader.lock();

        if (instanceData.find(modelName) == instanceData.end()) {
            instanceData[modelName] = std::make_shared<i_StaticInstanceData>(model, shader);

            if (instanceDataPerShader.find(shared->GetName()) == instanceDataPerShader.end()) {
                for (i_CommandBuffer &buffer: instanceDataPerShader[shared->GetName()].first) {
                    i_CommandBufferCreateInfo info{};

                    info.threadIndex = (currThreadIndex++) % std::thread::hardware_concurrency();
                    info.type = i_CommandBufferType::INSTANCE;
                    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                    info.flags = COMMAND_BUFFER_GRAPHICS_FLAG | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
                    //TODO i mean it will be
                    //updated often but is it worth it? do perfomrance tests

                    buffer = i_CommandBuffer(info);
                }
            }
            instanceDataPerShader[shared->GetName()].second.push_front(instanceData[modelName]);
        }

        instanceData[modelName]->AddInstance(instance);
    }


    void i_StaticInstanceManager::HandleCommandBuffers() {
        std::vector<std::thread> threads;
        threads.reserve(instanceDataPerShader.size());

        for (const auto &[shaderName, pair]: instanceDataPerShader) {
            threads.emplace_back(RecordCommandBuffer, i_ShaderManager::GetShader(shaderName), std::ref(pair.second), pair.first[i_Swapchain::GetFrameInFlight()]);
        }

        for (std::thread &thread: threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void i_StaticInstanceManager::RecordCommandBuffer(const ShaderHandle &shader,
                                                      const std::list<std::shared_ptr<i_StaticInstanceData> > &
                                                      instanceData,
                                                      i_CommandBuffer commandBuffer) {
        std::shared_ptr<i_Shader> shared = shader.lock();

        commandBuffer.BeginCommandBuffer(nullptr, false);
        shared->GetGraphicsPipeline()->BeginRenderPassAndBindPipeline(commandBuffer.GetCommandBuffer());

        for (const std::shared_ptr<i_StaticInstanceData> &data: instanceData) {
            data->UpdateAndRecordBuffer(commandBuffer);
        }

        shared->GetGraphicsPipeline()->EndRenderPass(commandBuffer.GetCommandBuffer());
        commandBuffer.EndCommandBuffer();
    }


    void i_StaticInstanceManager::Draw(const i_Semaphore& presentSemaphore) {
        std::vector<VkSemaphore> waitSemaphores;
        waitSemaphores.reserve(instanceData.size()+1);

        std::vector<VkPipelineStageFlags> waitDstStageMask;
        waitDstStageMask.reserve(instanceData.size()+1);

        std::vector<VkCommandBuffer> commandBuffers;
        commandBuffers.reserve(instanceDataPerShader.size());

        std::array<VkSemaphore, 1> signalSemaphores = {
            renderFinishedSemaphores[i_Swapchain::GetFrameInFlight()].GetSemaphore()
        };

        for (const auto &[model, data]: instanceData) {
            waitSemaphores.push_back(data->GetDataUploadedSemaphore().GetSemaphore());
            waitDstStageMask.push_back(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
        }
        waitSemaphores.push_back(presentSemaphore.GetSemaphore());
        waitDstStageMask.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        for (const auto &[shaderName, pair]: instanceDataPerShader) {
            commandBuffers.push_back(pair.first[i_Swapchain::GetFrameInFlight()].GetCommandBuffer());
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitDstStageMask.data();
        submitInfo.commandBufferCount = commandBuffers.size();
        submitInfo.pCommandBuffers = commandBuffers.data();
        submitInfo.pSignalSemaphores = signalSemaphores.data();
        submitInfo.signalSemaphoreCount = signalSemaphores.size();

        if (vkQueueSubmit(i_Device::GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit the static instance command buffer");
        }
    }


    VkSemaphore i_StaticInstanceManager::GetRenderFinishedSemaphore() {
        return renderFinishedSemaphores[i_Swapchain::GetFrameInFlight()].GetSemaphore();
    }
}

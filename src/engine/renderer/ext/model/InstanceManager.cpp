//
// Created by jakob on 8/9/24.
//

#include "InstanceManager.hpp"

namespace renderer {
    boost::container::flat_map<std::string, std::shared_ptr<i_InstanceData> >
    i_InstanceManager::instanceData = {};
    boost::container::flat_map<std::string, std::pair<std::array<i_CommandBuffer, MAX_FRAMES_IN_FLIGHT>,
        std::list<std::shared_ptr<i_InstanceData> > > >
    i_InstanceManager::instanceDataPerShader = {}; //shitty name but I couldnt give less than 2 shits

    uint32_t i_InstanceManager::currThreadIndex = 0;

    std::array<i_Semaphore, MAX_FRAMES_IN_FLIGHT> i_InstanceManager::renderFinishedSemaphores = {};

    void i_InstanceManager::Init() {
        for (i_Semaphore &semaphore: renderFinishedSemaphores) {
            i_SemaphoreCreateInfo info{};

            semaphore = i_Semaphore(info);
        }
    }

    void i_InstanceManager::EarlyUpdate() {
        UpdateDataBuffers();
    }

    void i_InstanceManager::UpdateDataBuffers() {
        std::list<std::thread> threads;

        for (auto &[shaderName, pair]: instanceDataPerShader) {
            for (std::shared_ptr<i_InstanceData> &instanceData: pair.second) {
                threads.emplace_back(std::bind(&i_InstanceData::UpdateDataBuffer, instanceData.get(),
                                               pair.first[i_Swapchain::GetFrameInFlight()].GetThreadIndex()));
            }
        }

        for (std::thread &thread: threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }


    void i_InstanceManager::Update() {
        HandleCommandBuffers();
    }

    void i_InstanceManager::Cleanup() {
        for (i_Semaphore &semaphore: renderFinishedSemaphores) {
            semaphore.Destruct();
        }

        instanceDataPerShader.clear();
        instanceData.clear();
    }

    void i_InstanceManager::AddInstance(const i_ModelInstanceHandleInternal &instance,
                                              const ShaderHandle &shader) {
        ModelHandle model = instance.lock()->GetModel();
        std::string modelName = model.lock()->GetName();
        std::shared_ptr<i_Shader> shared = shader.lock();

        if (instanceData.find(modelName) == instanceData.end()) {
            instanceData[modelName] = std::make_shared<i_InstanceData>(model, shader);

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


    void i_InstanceManager::HandleCommandBuffers() {
        std::vector<std::thread> threads;
        threads.reserve(instanceDataPerShader.size());

        for (auto &[shaderName, pair]: instanceDataPerShader) {
            threads.emplace_back(RecordCommandBuffer, i_ShaderManager::GetShader(shaderName), std::ref(pair.second),
                                 pair.first[i_Swapchain::GetFrameInFlight()]);
        }

        for (std::thread &thread: threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void i_InstanceManager::RecordCommandBuffer(const ShaderHandle &shader,
                                                      const std::list<std::shared_ptr<i_InstanceData> > &
                                                      instanceData,
                                                      i_CommandBuffer commandBuffer) {
        std::shared_ptr<i_Shader> shared = shader.lock();

        commandBuffer.BeginCommandBuffer(nullptr, false);
        shared->GetGraphicsPipeline()->BeginRenderPassAndBindPipeline(commandBuffer.GetCommandBuffer());

        for (const std::shared_ptr<i_InstanceData> &data: instanceData) {
            data->RecordCommandBuffer(commandBuffer);
            //do these things simoltaneously and figure out why the signal semaphore from the databuffer don'
        }

        shared->GetGraphicsPipeline()->EndRenderPass(commandBuffer.GetCommandBuffer());
        commandBuffer.EndCommandBuffer();
    }


    void i_InstanceManager::Draw(const i_Semaphore &presentSemaphore, const i_Fence& inFlightFence) {
        std::vector<VkSemaphore> waitSemaphores;
        if (i_Device::DeviceMemoryFree()) {
            waitSemaphores.reserve(instanceData.size() + 1);
        } else {
            waitSemaphores.reserve(2);
        }

        std::vector<VkPipelineStageFlags> waitDstStageMask;
        waitDstStageMask.reserve(waitSemaphores.size());

        std::vector<VkCommandBuffer> commandBuffers;
        commandBuffers.reserve(instanceDataPerShader.size());

        std::vector<uint64_t> waitSemaphoreValues;
        commandBuffers.reserve(instanceDataPerShader.size());

        std::array<VkSemaphore, 1> signalSemaphores = {
            renderFinishedSemaphores[i_Swapchain::GetFrameInFlight()].GetSemaphore()
        };

        if (i_Device::DeviceMemoryFree()) {
            for (const auto &[model, data]: instanceData) {
                waitSemaphores.push_back(data->GetDataUploadedSemaphore().GetSemaphore());
                waitDstStageMask.push_back(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
                waitSemaphoreValues.push_back(0);
            }
        }
        waitSemaphores.push_back(presentSemaphore.GetSemaphore());
        waitSemaphoreValues.push_back(0);
        waitDstStageMask.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        waitSemaphores.push_back(Camera::i_GetPerspectiveSignalSemaphore().GetSemaphore());
        waitSemaphoreValues.push_back(i_Swapchain::GetFrameCount()-1);
        waitDstStageMask.push_back(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);


        for (const auto &[shaderName, pair]: instanceDataPerShader) {
            commandBuffers.push_back(pair.first[i_Swapchain::GetFrameInFlight()].GetCommandBuffer());
        }

        VkTimelineSemaphoreSubmitInfo timelineSubmitInfo{};
        timelineSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;

        timelineSubmitInfo.signalSemaphoreValueCount = 0;

        timelineSubmitInfo.pWaitSemaphoreValues = waitSemaphoreValues.data();
        timelineSubmitInfo.waitSemaphoreValueCount = waitSemaphores.size();


        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = &timelineSubmitInfo;

        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitDstStageMask.data();
        submitInfo.commandBufferCount = commandBuffers.size();
        submitInfo.pCommandBuffers = commandBuffers.data();
        submitInfo.pSignalSemaphores = signalSemaphores.data();
        submitInfo.signalSemaphoreCount = signalSemaphores.size();

        if (vkQueueSubmit(i_Device::GetGraphicsQueue(), 1, &submitInfo, inFlightFence.GetFence()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit the static instance command buffer");
        }
    }


    VkSemaphore i_InstanceManager::GetRenderFinishedSemaphore() {
        return renderFinishedSemaphores[i_Swapchain::GetFrameInFlight()].GetSemaphore();
    }
}

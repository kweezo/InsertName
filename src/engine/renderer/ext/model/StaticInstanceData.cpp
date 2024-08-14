//
// Created by jakob on 8/8/24.
//

#include "StaticInstanceData.hpp"

#include <utility>

namespace renderer {
    i_StaticInstanceData::i_StaticInstanceData(ModelHandle model, ShaderHandle shader): model(std::move(
            model)),
        shader(std::move(shader)), buffer(), dataUploadedSemaphore(),
        drawCount(0), instances() {
        constexpr i_SemaphoreCreateInfo createInfo{};
        dataUploadedSemaphore = i_Semaphore(createInfo);
    }

    void i_StaticInstanceData::AddInstance(const i_ModelInstanceHandleInternal &instance) {
        instances.push_front(instance);
    }

    void i_StaticInstanceData::UpdateDataBuffer(uint32_t threadIndex) {
        GetDrawableInstances();
        UploadDataToBuffer(threadIndex);
    }

    void i_StaticInstanceData::GetDrawableInstances() {
        std::list<glm::mat4> transformInstanceDataLinked;

        auto iteratorInstances = instances.begin();
        for (i_ModelInstanceHandleInternal &instance: instances) {
            if (instance.expired()) {
                instances.erase(iteratorInstances);
                std::advance(iteratorInstances, 1);

                continue;
            }

            const ModelInstanceHandle shared = instance.lock();

            if (!shared->GetShouldDraw()) {
                continue;
            }

            transformInstanceDataLinked.push_front(shared->GetModelMatrix());

            ++drawCount;


            std::advance(iteratorInstances, 1);
        }


        transformInstanceData.resize(transformInstanceDataLinked.size());

        auto iteratorInstanceDataLinked = transformInstanceDataLinked.begin();
        for (auto &i: transformInstanceData) {
            i = *iteratorInstanceDataLinked;

            std::advance(iteratorInstanceDataLinked, 1);
        }
    }

    void i_StaticInstanceData::UploadDataToBuffer(uint32_t threadIndex) {
        i_DataBufferCreateInfo createInfo{};

        createInfo.data = transformInstanceData.data();
        createInfo.size = transformInstanceData.size();
        createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        createInfo.isDynamic = false;
        createInfo.transferToLocalDeviceMemory = true;
        createInfo.threadIndex = threadIndex;
        if(i_Device::DeviceMemoryFree()) {
            createInfo.signalSemaphore = dataUploadedSemaphore;
        }

        buffer = i_DataBuffer(createInfo);
    }

    void i_StaticInstanceData::RecordCommandBuffer(i_CommandBuffer commandBuffer) {
        std::shared_ptr<i_Shader> shader = this->shader.lock();
        std::shared_ptr<i_Model> model = this->model.lock();

        VkBuffer vertexBuffer = buffer.GetBuffer();
        VkDeviceSize offset = 0;

        VkDescriptorSet set = shader->GetDescriptorSet();

        vkCmdBindVertexBuffers(commandBuffer.GetCommandBuffer(), 1, 1, &vertexBuffer,
                               &offset);

        vkCmdBindDescriptorSets(commandBuffer.GetCommandBuffer(),
                                VK_PIPELINE_BIND_POINT_GRAPHICS, shader->GetGraphicsPipeline()->GetPipelineLayout(), 0,
                                1, &set, 0, nullptr);

        //model->GetExtraDrawCommands()(); TODO no crashy crashy

        model->RecordDrawCommands(commandBuffer, drawCount);
    }

    i_Semaphore i_StaticInstanceData::GetDataUploadedSemaphore() const{
        return dataUploadedSemaphore;
    }
}
#include "UniformBuffer.hpp"


namespace renderer {
    std::vector<VkWriteDescriptorSet> i_UniformBuffer::writeDescriptorSetsQueue = {};
    std::list<VkDescriptorBufferInfo> i_UniformBuffer::bufferInfoList = {};

    void i_UniformBuffer::Update() {
        vkUpdateDescriptorSets(i_Device::GetDevice(), writeDescriptorSetsQueue.size(), writeDescriptorSetsQueue.data(),
                               0, nullptr);
        writeDescriptorSetsQueue.clear();
        bufferInfoList.clear();
    }

    i_UniformBuffer::i_UniformBuffer(): size(0), binding(-1) {
    }

    i_UniformBuffer::i_UniformBuffer(const i_UniformBufferCreateInfo &createInfo): size(createInfo.size),
        binding(createInfo.binding),
        shaders(createInfo.shaders) {
        i_DataBufferCreateInfo dataBufferCreateInfo{};
        dataBufferCreateInfo.size = size;
        dataBufferCreateInfo.data = createInfo.data;
        dataBufferCreateInfo.transferToLocalDeviceMemory = true;
        dataBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        dataBufferCreateInfo.isDynamic = true;

        dataBuffer = i_DataBuffer(dataBufferCreateInfo);

        SetBinding(binding);

        useCount = std::make_shared<uint32_t>(1);
    }

    void i_UniformBuffer::UpdateData(void *data, size_t size, uint32_t threadIndex) {
        if (this->size != size) {
            throw std::runtime_error("Data size mismatch when trying to update uniform buffer data, expected size: " +
                                     std::to_string(this->size) + " actual size: " + std::to_string(size));
        }

        dataBuffer.UpdateData(data, size, threadIndex);
    }

    void i_UniformBuffer::SetBinding(uint32_t binding) {
        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.descriptorCount = 1;

        static VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = dataBuffer.GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = size;

        bufferInfoList.push_front(bufferInfo);

        writeDescriptorSet.pBufferInfo = &bufferInfoList.front();

        for (const ShaderHandle &shader: shaders) {
            writeDescriptorSet.dstSet = shader.lock()->GetDescriptorSet();
            writeDescriptorSetsQueue.push_back(writeDescriptorSet);
        }

        this->binding = binding;
    }

    i_UniformBuffer::i_UniformBuffer(const i_UniformBuffer &other) {
        if (other.useCount == nullptr) {
            return;
        }

        dataBuffer = other.dataBuffer;
        size = other.size;
        binding = other.binding;
        shaders = other.shaders;

        useCount = other.useCount;
        (*useCount)++;
    }

    i_UniformBuffer &i_UniformBuffer::operator=(const i_UniformBuffer &other) {
        if (this == &other) {
            return *this;
        }

        if (other.useCount == nullptr) {
            return *this;
        }

        Destructor();

        dataBuffer = other.dataBuffer;
        size = other.size;
        binding = other.binding;
        shaders = other.shaders;

        useCount = other.useCount;
        (*useCount)++;

        return *this;
    }

    void i_UniformBuffer::Destructor() {
        if (useCount == nullptr) {
            return;
        }

        if (*useCount <= 1) {
            dataBuffer.Destruct();

            useCount.reset();

            return;
        }

        (*useCount)--;
    }

    i_UniformBuffer::~i_UniformBuffer() {
        Destructor();
    }
}

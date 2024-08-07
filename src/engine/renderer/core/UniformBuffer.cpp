#include "UniformBuffer.hpp"


namespace renderer{

__UniformBuffer::__UniformBuffer(){
    useCount = std::make_shared<uint32_t>(1);
}

__UniformBuffer::__UniformBuffer(__UniformBufferCreateInfo createInfo): size(createInfo.size), binding(createInfo.binding), 
descriptorSet(createInfo.descriptorSet) {

    __DataBufferCreateInfo dataBufferCreateInfo{};
    dataBufferCreateInfo.size = size;
    dataBufferCreateInfo.data = createInfo.data;
    dataBufferCreateInfo.transferToLocalDeviceMemory = true;
    dataBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    dataBufferCreateInfo.isDynamic = true;

    dataBuffer = __DataBuffer(dataBufferCreateInfo);

    useCount = std::make_shared<uint32_t>(1);
}

void __UniformBuffer::SetDescriptorSet(VkDescriptorSet descriptorSet){
    this->descriptorSet = descriptorSet;
}

void __UniformBuffer::SetBinding(uint32_t binding){
    this->binding = binding;
}

void __UniformBuffer::UpdateData(void* data, size_t size, uint32_t threadIndex){
    if(this->size != size){
        throw std::runtime_error("Data size mismatch when trying to update uniform buffer data, expected size: " +
         std::to_string(this->size) + " actual size: " + std::to_string(size));
    }

    dataBuffer.UpdateData(data, size, threadIndex);
}

VkWriteDescriptorSet __UniformBuffer::GetWriteDescriptorSet(){
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.descriptorCount = 1;

    static VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = dataBuffer.GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = size;

    writeDescriptorSet.pBufferInfo = &bufferInfo;

    return writeDescriptorSet;
}

__UniformBuffer::__UniformBuffer(const __UniformBuffer& other){
    dataBuffer = other.dataBuffer;
    size = other.size;
    binding = other.binding;
    descriptorSet = other.descriptorSet;

    useCount = other.useCount;
    (*useCount.get())++;
}

__UniformBuffer  __UniformBuffer::operator=(const __UniformBuffer& other){
    if(this == &other){
        return *this;
    }

    dataBuffer = other.dataBuffer;
    size = other.size;
    binding = other.binding;
    descriptorSet = other.descriptorSet;

    useCount = other.useCount;
    (*useCount.get())++;

    return *this;
}
__UniformBuffer::~__UniformBuffer(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount.get() == 1){
        dataBuffer.~__DataBuffer();

        useCount.reset();

        return;
    }

    (*useCount.get())--;
}


}
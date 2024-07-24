#include "UniformBuffer.hpp"


namespace renderer{

std::vector<VkWriteDescriptorSet> _UniformBuffer::writeDescriptorSetsQueue = {};

void _UniformBuffer::Update(){
    if(!writeDescriptorSetsQueue.empty()){
        vkUpdateDescriptorSets(_Device::GetDevice(), writeDescriptorSetsQueue.size(), writeDescriptorSetsQueue.data(),
        0, nullptr);
    }
}

_UniformBuffer::_UniformBuffer(){

}

_UniformBuffer::_UniformBuffer(_UniformBufferCreateInfo createInfo): size(createInfo.size), binding(createInfo.binding), 
descriptorSet(createInfo.descriptorSet) {

    _DataBufferCreateInfo dataBufferCreateInfo{};
    dataBufferCreateInfo.size = size;
    dataBufferCreateInfo.data = createInfo.data;
    dataBufferCreateInfo.transferToLocalDeviceMemory = true;
    dataBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    dataBufferCreateInfo.isDynamic = true;

    dataBuffer = _DataBuffer(dataBufferCreateInfo);

    UpdateDescriptorSet(binding);

    useCount = std::make_shared<uint32_t>(1);
}

void _UniformBuffer::SetDescriptorSet(VkDescriptorSet descriptorSet){
    this->descriptorSet = descriptorSet;
}

void _UniformBuffer::SetBinding(uint32_t binding){
    this->binding = binding;
}

void _UniformBuffer::UpdateData(void* data, size_t size, uint32_t threadIndex){
    if(this->size != size){
        throw std::runtime_error("Data size mismatch when trying to update uniform buffer data, expected size: " +
         std::to_string(this->size) + " actual size: " + std::to_string(size));
    }

    dataBuffer.UpdateData(data, size, threadIndex);
}

VkWriteDescriptorSet _UniformBuffer::GetWriteDescriptorSet(){
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

void _UniformBuffer::UpdateDescriptorSet(uint32_t binding){
    this->binding = binding;
    writeDescriptorSetsQueue.push_back(GetWriteDescriptorSet());
}

VkDescriptorSet _UniformBuffer::GetDescriptorSet(){
    return descriptorSet;
}

_UniformBuffer::_UniformBuffer(const _UniformBuffer& other){
    if(other.useCount.get() == nullptr){
        return;
    }

    dataBuffer = other.dataBuffer;
    size = other.size;
    binding = other.binding;
    descriptorSet = other.descriptorSet;

    useCount = other.useCount;
    (*useCount.get())++;
}

_UniformBuffer&  _UniformBuffer::operator=(const _UniformBuffer& other){
    if(this == &other){
        return *this;
    }

    if(other.useCount.get() == nullptr){
        return *this;
    }

    Destructor();

    dataBuffer = other.dataBuffer;
    size = other.size;
    binding = other.binding;
    descriptorSet = other.descriptorSet;

    useCount = other.useCount;
    (*useCount.get())++;

    return *this;
}

void _UniformBuffer::Destructor(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount.get() <= 1){
        dataBuffer.~_DataBuffer();

        useCount.reset();

        return;
    }

    (*useCount.get())--;
}

_UniformBuffer::~_UniformBuffer(){
    Destructor();
}


}
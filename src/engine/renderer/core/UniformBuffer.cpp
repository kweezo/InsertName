#include "UniformBuffer.hpp"


namespace renderer{

std::vector<VkWriteDescriptorSet> _UniformBuffer::writeDescriptorSetsQueue = {};
std::list<VkDescriptorBufferInfo> _UniformBuffer::bufferInfoList = {};

void _UniformBuffer::Update(){
    vkUpdateDescriptorSets(_Device::GetDevice(), writeDescriptorSetsQueue.size(), writeDescriptorSetsQueue.data(),
    0, nullptr);
    writeDescriptorSetsQueue.clear();
    bufferInfoList.clear();
}

_UniformBuffer::_UniformBuffer(){

}

_UniformBuffer::_UniformBuffer(_UniformBufferCreateInfo createInfo): size(createInfo.size), binding(createInfo.binding), 
shaders(createInfo.shaders) {

    _DataBufferCreateInfo dataBufferCreateInfo{};
    dataBufferCreateInfo.size = size;
    dataBufferCreateInfo.data = createInfo.data;
    dataBufferCreateInfo.transferToLocalDeviceMemory = true;
    dataBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    dataBufferCreateInfo.isDynamic = true;

    dataBuffer = _DataBuffer(dataBufferCreateInfo);

    SetBinding(binding);

    useCount = std::make_shared<uint32_t>(1);
}

void _UniformBuffer::UpdateData(void* data, size_t size, uint32_t threadIndex){
    if(this->size != size){
        throw std::runtime_error("Data size mismatch when trying to update uniform buffer data, expected size: " +
         std::to_string(this->size) + " actual size: " + std::to_string(size));
    }

    dataBuffer.UpdateData(data, size, threadIndex);
}

void _UniformBuffer::SetBinding(uint32_t binding){
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

    for(std::weak_ptr<_Shader> shader : shaders){
        writeDescriptorSet.dstSet = shader.lock()->GetDescriptorSet();
        writeDescriptorSetsQueue.push_back(writeDescriptorSet);
    }

    this->binding = binding;
}

_UniformBuffer::_UniformBuffer(const _UniformBuffer& other){
    if(other.useCount.get() == nullptr){
        return;
    }

    dataBuffer = other.dataBuffer;
    size = other.size;
    binding = other.binding;
    shaders = other.shaders;

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
    shaders = other.shaders;

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
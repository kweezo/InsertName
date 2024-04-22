#include "UniformBuffer.hpp"

namespace renderer{



UniformBufferHandle UniformBuffer::Create(void* data, size_t size, uint32_t binding, VkDescriptorSet descriptorSet){
    return new UniformBufferImpl(data, size, binding, descriptorSet);
}

void UniformBuffer::Free(UniformBufferHandle buffer){
    delete buffer;
}

UniformBufferImpl::UniformBufferImpl(void* data, size_t size, uint32_t binding, VkDescriptorSet descriptorSet){
    BufferDescriptions descriptions{};

    dataBuffer = DataBuffer({}, size, data, true, DATA_BUFFER_UNIFORM_BIT);

    dataSize = size;
    this->binding = binding;
    this->descriptorSet = descriptorSet;
}

void UniformBufferImpl::SetDescriptorSet(VkDescriptorSet descriptorSet){
    this->descriptorSet = descriptorSet; 
}

void UniformBufferImpl::UpdateData(void* data, size_t size){
    if(dataSize != size){
        throw std::runtime_error("Data size mismatch when trying to update uniform buffer data, expected size: " +
         std::to_string(dataSize) + " actual size: " + std::to_string(size));
    }

    dataBuffer.UpdateData(data, size);
}

VkWriteDescriptorSet UniformBufferImpl::GetWriteDescriptorSet(){
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
    bufferInfo.range = dataSize;

    writeDescriptorSet.pBufferInfo = &bufferInfo;

    return writeDescriptorSet;
}

void UniformBufferImpl::AssignDescriptorHandle(DescriptorHandle handle){
    descriptorHandle = handle;
}


}


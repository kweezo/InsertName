#include "UniformBuffer.hpp"

namespace renderer{


struct DescriptorCountMapInfo{
    UniformBufferCreateInfo createInfo;
    uint32_t count = 0;
    std::vector<UniformBufferImpl*> uniformHandles = {};
};

bool UniformBufferImpl::creationLock = false;
std::unordered_map<std::string, DescriptorCountMapInfo> descriptorCountPerLayout = {};

UniformBufferHandle UniformBuffer::Create(void* data, size_t size, UniformBufferCreateInfo info){
    return new UniformBufferImpl(data, size, info);
}

void UniformBuffer::EnableBuffers(){
    UniformBufferImpl::EnableBuffers();
}

void UniformBuffer::Free(UniformBufferHandle buffer){
    delete buffer;
}

UniformBufferImpl::UniformBufferImpl(void* data, size_t size, UniformBufferCreateInfo info){
    if(creationLock){
        throw std::runtime_error("Tried to create a buffer after enabling them, \n which is thus far not allowed.");
    }

    BufferDescriptions descriptions{};

    dataBuffer = DataBuffer({}, size, data, true, DATA_BUFFER_UNIFORM_BIT);

    if(descriptorCountPerLayout.find(info.name) != descriptorCountPerLayout.end()){
        descriptorCountPerLayout[info.name].count++;
        descriptorCountPerLayout[info.name].uniformHandles.push_back(this);
    }else{
        descriptorCountPerLayout.insert({info.name, {info, 1, {this}}});
    }
    
    dataSize = size;
    name = info.name;
}

void UniformBufferImpl::UpdateData(void* data, size_t size){
    if(dataSize != size){
        throw std::runtime_error("Data size mismatch when trying to update uniform buffer data, expected size: " +
         std::to_string(dataSize) + " actual size: " + std::to_string(size));
    }

    dataBuffer.UpdateData(data, size);

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = DescriptorManager::GetDescriptorSet(descriptorHandle);
    writeDescriptorSet.dstBinding = descriptorCountPerLayout[name].createInfo.index;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.descriptorCount = 1;

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = dataBuffer.GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = size;

    writeDescriptorSet.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(Device::GetDevice(), 1, &writeDescriptorSet, 0, nullptr);
}

void UniformBufferImpl::ForceWriteDescriptorSet(){
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = DescriptorManager::GetDescriptorSet(descriptorHandle);
    writeDescriptorSet.dstBinding = descriptorCountPerLayout[name].createInfo.index;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.descriptorCount = 1;

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = dataBuffer.GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = dataSize;

    writeDescriptorSet.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(Device::GetDevice(), 1, &writeDescriptorSet, 0, nullptr);
}

void UniformBufferImpl::AssignDescriptorHandle(DescriptorHandle handle){
    descriptorHandle = handle;
}

void UniformBufferImpl::EnableBuffers(){
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = descriptorCountPerLayout.size();
    std::vector<VkDescriptorSetLayoutBinding> bindings(descriptorCountPerLayout.size());
    layoutInfo.pBindings = bindings.data();
    uint32_t i = 0;
    for(auto& [name, descriptorCountMapInfo] : descriptorCountPerLayout){
        bindings[i].binding = descriptorCountMapInfo.createInfo.index;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[i].descriptorCount = descriptorCountMapInfo.count;
        bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //TODO make this configurable
        i++;    
    }


    std::vector<uint32_t> layoutIndices = DescriptorManager::CreateLayouts({layoutInfo});


    DescriptorBatchInfo batchInfo = {};
    batchInfo.layoutIndex = reinterpret_cast<uint32_t*>(layoutIndices.data())[0];
    batchInfo.setCount = descriptorCountPerLayout.size();

    std::vector<DescriptorHandle> descriptorBatch = DescriptorManager::CreateDescriptors({batchInfo});

    for (auto& [name, descriptorCountMapInfo] : descriptorCountPerLayout){
        for(UniformBufferImpl* buffer : descriptorCountMapInfo.uniformHandles){
            buffer->AssignDescriptorHandle(descriptorBatch[0]);

            buffer->ForceWriteDescriptorSet();

        }
    }

}

void UniformBufferImpl::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout layout){
    VkDescriptorSet descriptorSet = DescriptorManager::GetDescriptorSet(descriptorHandle);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, nullptr);

}

}


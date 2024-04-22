#pragma once


#include <vector>
#include <string>
#include <stdexcept>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "DescriptorManager.hpp"

namespace renderer{


class UniformBufferImpl{
public:
    UniformBufferImpl(void* data, size_t size, uint32_t binding, VkDescriptorSet descriptorSet);

    void UpdateData(void* data, size_t size);
    void SetDescriptorSet(VkDescriptorSet descriptorSet);

    VkWriteDescriptorSet GetWriteDescriptorSet();

    void AssignDescriptorHandle(DescriptorHandle handle);// DO NOT TOUCH THIS UNLESS YOU KNOW WHY ITS LIKE THIS,
    // NO HARAM
private:
    DataBuffer dataBuffer;
    DescriptorHandle descriptorHandle;

    VkDescriptorSet descriptorSet;

    size_t dataSize;
    uint32_t binding;

};

#define UniformBufferHandle UniformBufferImpl*


class UniformBuffer{
public:
    static UniformBufferHandle Create(void* data, size_t size, uint32_t binding, VkDescriptorSet descriptorSet);
    static void Free(UniformBufferHandle buffer);

    static void EnableBuffers();
};

}
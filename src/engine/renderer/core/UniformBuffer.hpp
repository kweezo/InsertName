#pragma once


#include <vector>
#include <string>
#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "DescriptorManager.hpp"


namespace renderer{

struct __UniformBufferCreateInfo{
    VkDescriptorSet descriptorSet;
    uint32_t binding;
    uint32_t threadIndex;

    void* data;
    size_t size;
};


class __UniformBuffer{
public:
    __UniformBuffer();
    __UniformBuffer(__UniformBufferCreateInfo createInfo);

    __UniformBuffer(const __UniformBuffer& other);
    __UniformBuffer operator=(const __UniformBuffer& other);
    ~__UniformBuffer();


    void UpdateData(void* data, size_t size, uint32_t threadIndex);
    void SetDescriptorSet(VkDescriptorSet descriptorSet);
    void SetBinding(uint32_t binding);

    VkWriteDescriptorSet GetWriteDescriptorSet();
private:

    __DataBuffer dataBuffer;

    size_t size;
    uint32_t binding;
    VkDescriptorSet descriptorSet;

    std::shared_ptr<uint32_t> useCount;
};

}
#pragma once


#include <vector>
#include <string>
#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "DescriptorManager.hpp"


namespace renderer{

struct _UniformBufferCreateInfo{
    VkDescriptorSet descriptorSet;
    uint32_t binding;
    uint32_t threadIndex;

    void* data;
    size_t size;
};


class _UniformBuffer{
public:
    _UniformBuffer();
    _UniformBuffer(_UniformBufferCreateInfo createInfo);

    _UniformBuffer(const _UniformBuffer& other);
    _UniformBuffer operator=(const _UniformBuffer& other);
    ~_UniformBuffer();


    void UpdateData(void* data, size_t size, uint32_t threadIndex);
    void SetDescriptorSet(VkDescriptorSet descriptorSet);
    void SetBinding(uint32_t binding);

    VkWriteDescriptorSet GetWriteDescriptorSet();
private:
    void Destructor();

    _DataBuffer dataBuffer;

    size_t size;
    uint32_t binding;
    VkDescriptorSet descriptorSet;

    std::shared_ptr<uint32_t> useCount;
};

}
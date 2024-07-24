#pragma once


#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <tuple>
#include <thread>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "DescriptorManager.hpp"
#include "CommandBuffer.hpp"

#define UNIFORM_BUFFER_COMMAND_BUFFERS_PER_THREAD 1

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
    static void Update();

    _UniformBuffer();
    _UniformBuffer(_UniformBufferCreateInfo createInfo);

    _UniformBuffer(const _UniformBuffer& other);
    _UniformBuffer operator=(const _UniformBuffer& other);
    ~_UniformBuffer();

    void UpdateDescriptorSet(uint32_t binding);

    void UpdateData(void* data, size_t size, uint32_t threadIndex);
    void SetDescriptorSet(VkDescriptorSet descriptorSet);
    void SetBinding(uint32_t binding);

    VkDescriptorSet GetDescriptorSet();

    VkWriteDescriptorSet GetWriteDescriptorSet();
private:
    void Destructor();

    _DataBuffer dataBuffer;

    size_t size;
    uint32_t binding;
    VkDescriptorSet descriptorSet;

    std::shared_ptr<uint32_t> useCount;

    static std::vector<VkWriteDescriptorSet> writeDescriptorSetsQueue;
};

}
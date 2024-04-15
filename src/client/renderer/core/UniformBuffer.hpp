#pragma once


#include <vector>
#include <stdexcept>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "DescriptorManager.hpp"

namespace renderer{

struct UniformBufferCreateInfo{
    std::string name;
    uint32_t index;
};


class UniformBuffer{
public:
    UniformBuffer(void* data, size_t size, UniformBufferCreateInfo info);

    void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout layout);

    void UpdateData(void* data, size_t size);
    void ForceWriteDescriptorSet();

    static void EnableBuffers();

    void AssignDescriptorHandle(DescriptorHandle handle);// DO NOT TOUCH THIS UNLESS YOU KNOW WHY ITS LIKE THIS,
    // NO HARAM
private:
    DataBuffer dataBuffer;
    DescriptorHandle descriptorHandle;

    size_t dataSize;
    std::string name;
    uint32_t layoutIndex;

    static bool creationLock;

};

}
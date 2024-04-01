#pragma once

#include <vector>
#include <stdexcept>
#include <cstring>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"

namespace renderer{

class Buffer{
public:
    VkBuffer GetBuffer();
protected:
    static void AllocateMemory(VkDeviceMemory& memory, VkBuffer buffer, size_t size, VkMemoryPropertyFlags properties);
    static void CreateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceSize size);
    
    VkBuffer buff;
    VkDeviceMemory mem;
};

}
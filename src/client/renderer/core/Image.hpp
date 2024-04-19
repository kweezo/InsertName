#pragma once

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "Device.hpp"
#include "CommandBuffer.hpp"

namespace renderer{

class Image{
public:
    Image();
    Image(VkImageLayout layout, VkFormat format, uint32_t width,
     uint32_t height, size_t size, void* data);

    VkImage GetImage();
private:
    VkImage image;
    VkDeviceMemory memory;
};
}
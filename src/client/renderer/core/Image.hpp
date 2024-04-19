#pragma once

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "Device.hpp"

namespace renderer{

class Image{
public:
    Image(VkImageLayout layout, VkFormat format, uint32_t width,
     uint32_t height, size_t size, void* data);

    VkImage GetImage();
private:
    VkImage image;
};
}
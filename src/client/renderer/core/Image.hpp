#pragma once

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "Device.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"

namespace renderer{

typedef struct ImageTransitionCMDInfo{
    CommandBuffer commandBuffer;
    bool free;
} ImageCopyCMDInfo;

typedef struct LoadDataIntoImageInfo{
    VkImage image;
    VkDeviceMemory memory;
    VkFormat format;
    uint32_t width;
    uint32_t height;
    size_t size;
    void* data;
} LoadDataIntoImageInfo;


class Image{
public:
    Image();
    Image(VkImageLayout layout, VkFormat format, uint32_t width,
     uint32_t height, size_t size, void* data);

    static void Initialize();

    VkImage GetImage();

    static void UpdateCommandBuffers();

    void TransitionImageLayout();
private:
    static void CreateCommandBuffers();
    static CommandBuffer GetFreeCommandBuffer();

    static std::vector<ImageTransitionCMDInfo> stagingBuffers;
    static Fence finishedTransitioningFence;
    static CommandBuffer primaryCommandBuffer;


    VkImage image;
    VkDeviceMemory memory;
};
}
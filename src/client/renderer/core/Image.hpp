#pragma once

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "Device.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"

namespace renderer{class ImageImpl;}

#define ImageHandle renderer::ImageImpl*

namespace renderer{

typedef struct ImageTransitionCMDInfo{
    CommandBuffer commandBuffer;
    bool free;
    ImageHandle image;
} ImageCopyCMDInfo;

typedef struct LoadDataIntoImageInfo{
    VkImage image;
    VkDeviceMemory memory;
    size_t size;
    void* data;
    VkExtent3D extent;
    VkImageSubresourceLayers subresource;
} LoadDataIntoImageInfo;


class Image{
public:

    static ImageHandle CreateImage(VkImageLayout layout, VkFormat format, uint32_t width,
     uint32_t height, size_t size, void* data);

    static void Free(ImageHandle image);

    static void Initialize();
    static void UpdateCommandBuffers();
};

class ImageImpl{
public:
    ImageImpl();
    ImageImpl(VkImageLayout layout, VkFormat format, uint32_t width,
     uint32_t height, size_t size, void* data);

    static void Initialize();

    VkImage GetImage();

    static void UpdateCommandBuffers();

    void TransitionImageLayout();
private:
    static void CreateCommandBuffers();
    static CommandBuffer GetFreeCommandBuffer(ImageHandle image);

    static std::vector<ImageTransitionCMDInfo> stagingBuffers;
    static Fence finishedTransitioningFence;
    static CommandBuffer primaryCommandBuffer;


    LoadDataIntoImageInfo loadDataInfo;

    VkImage image;
    VkDeviceMemory memory;
};
}
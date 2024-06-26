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
    VkImageLayout layout;
} LoadDataIntoImageInfo;


class Image{
public:

    static ImageHandle CreateImage(VkImageLayout layout, VkFormat format, VkImageAspectFlags aspectMask, VkImageUsageFlags usage,
     uint32_t width, uint32_t height, size_t size, void* data);

    static void Free(ImageHandle image);

    static void Initialize();
    static void UpdateCommandBuffers();

    static VkFormat GetSupportedFormat(std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    static inline bool HasStencilComponent(VkFormat format);
};

class ImageImpl{
public:
    ImageImpl();
    ImageImpl(VkImageLayout layout, VkFormat format, VkImageAspectFlags aspectMask, uint32_t width, VkImageUsageFlags usage,
     uint32_t height, size_t size, void* data);

    ~ImageImpl();
    ImageImpl(const ImageImpl& other);
    ImageImpl&  operator=(const ImageImpl& other);

    static void Initialize();
    static void Cleanup();

    VkImage GetImage();
    void LoadDataIntoImage();
    void TransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

    VkImageView GetImageView();

    static void UpdateCommandBuffers();
private:
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
    void CreateImageView(VkFormat format, VkImageAspectFlags aspectMask);

    static void CreateCommandBuffers();
    static CommandBuffer GetFreeCommandBuffer(ImageHandle image);

    static std::vector<ImageTransitionCMDInfo> stagingBuffers;
    static Fence finishedTransitioningFence;
    static CommandBuffer primaryCommandBuffer;

    uint32_t* useCount;

    LoadDataIntoImageInfo loadDataInfo;

    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;

    VkImageAspectFlags aspectMask;
};
}
#pragma once

#include <stdexcept>
#include <vector>
#include <memory>
#include <utility>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "Device.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"

namespace renderer{

struct ImageCreateInfo{
    VkImageLayout layout;
    VkFormat format;
    VkImageAspectFlags aspectMask;
    VkImageUsageFlags usage;
    VkExtent2D imageExtent;

    size_t size;
    void* data;

    uint32_t threadIndex;
};

class Image{
public:
    static void Init();
    static void Update();
    static void Cleanup();

    Image();
    Image(ImageCreateInfo createInfo);
    Image operator=(const Image& other);
    Image(const Image& other);
    ~Image();

    static VkFormat GetSupportedFormat(std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    static inline bool HasStencilComponent(VkFormat format);

    void TransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

    VkImage GetImage();
    VkImageView GetImageView();

private:
    void CreateImage();
    void CreateImageView();
    void AllocateMemory();


    static CommandBuffer GetFreeCommandBuffer(uint32_t threadIndex);
    static std::vector<std::vector<std::pair<CommandBuffer, bool>>> secondaryCommandBuffers;

    static void RecordPrimaryCommandBuffer();
    static void SubmitPrimaryCommandBuffer();
    static void UpdateCleanup();

    static void CreateCommmandBuffers();

    static CommandBuffer primaryCommandBuffer;
    static Fence finishedPrimaryCommandBufferExecutionFence;

    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;

    ImageCreateInfo createInfo;


    std::shared_ptr<uint32_t> useCount;
};
    
} 

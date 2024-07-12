#pragma once

#include <stdexcept>
#include <vector>
#include <memory>
#include <utility>
#include <set>
#include <list>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "Device.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"

namespace renderer{

struct __ImageCreateInfo{
    VkImageLayout layout;
    VkFormat format;
    VkImageAspectFlags aspectMask;
    VkImageUsageFlags usage;
    VkExtent2D imageExtent;

    bool copyToLocalDeviceMemory;

    size_t size;
    void* data;

    uint32_t threadIndex;
};

class __Image{
public:
    static void Init();
    static void Update();
    static void Cleanup();

    __Image();
    __Image(__ImageCreateInfo createInfo);
    __Image operator=(const __Image& other);
    __Image(const __Image& other);
    ~__Image();

    static VkFormat GetSupportedFormat(std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    static inline bool HasStencilComponent(VkFormat format);

    void TransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

    VkImage GetImage();
    VkImageView GetImageView();

private:
    void CreateImage();
    void CreateImageView();
    void AllocateMemory();

    void CopyDataToDevice();

    static __CommandBuffer GetFreeCommandBuffer(uint32_t threadIndex);
    static std::vector<std::vector<std::pair<__CommandBuffer, bool>>> secondaryCommandBuffers;

    static void SubmitCommandBuffers();
    static void UpdateCleanup();

    static void CreateCommmandBuffers();

    static __Fence commandBuffersFinishedExecutionFence;
    static std::set<uint32_t> commandPoolResetIndexes;
    static bool primaryCommandBufferRecorded;

    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    __ImageCreateInfo createInfo;

    static std::list<std::pair<VkBuffer, VkDeviceMemory>> bufferAndMemoryCleanupQueue;


    std::shared_ptr<uint32_t> useCount;
};
    
} 

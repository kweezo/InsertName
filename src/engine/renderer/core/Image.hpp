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
#include "Shader.hpp"

namespace renderer{

struct _ImageCreateInfo{
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

class _Image{
public:
    static void Init();
    static void Update();
    static void Cleanup();

    _Image();
    _Image(_ImageCreateInfo createInfo);
    _Image operator=(const _Image& other);
    _Image(const _Image& other);
    ~_Image();


    static VkFormat GetSupportedFormat(std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    static inline bool HasStencilComponent(VkFormat format);

    void TransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

    VkImage GetImage();
    VkImageView GetImageView();

private:
    void Destruct();

    void CreateImage();
    void CreateImageView();
    void AllocateMemory();

    void CopyDataToDevice();

    static _CommandBuffer GetFreeCommandBuffer(uint32_t threadIndex);
    static std::vector<std::vector<std::pair<_CommandBuffer, bool>>> secondaryCommandBuffers;

    static void SubmitCommandBuffers();
    static void UpdateCleanup();

    static void CreateCommmandBuffers();

    static _Fence commandBuffersFinishedExecutionFence;
    static std::set<uint32_t> commandPoolResetIndexes;
    static bool primaryCommandBufferRecorded;


    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;

    std::unique_ptr<_DataBuffer> stagingBuffer;

    _Semaphore waitSemaphore;

    _ImageCreateInfo createInfo;

    static std::list<std::unique_ptr<_DataBuffer>> bufferCleanupQueue;


    std::shared_ptr<uint32_t> useCount;
};
    
} 

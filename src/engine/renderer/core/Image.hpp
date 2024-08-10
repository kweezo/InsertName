#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <set>
#include <list>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"

namespace renderer {
    class i_Shader; //IM SORRY OKAY I HAD TO or else the dependencies go WEEEEEEEEEWOOOOOWEEEEWOOOO

    struct i_ImageCreateInfo {
        VkImageLayout layout;
        VkFormat format;
        VkImageAspectFlags aspectMask;
        VkImageUsageFlags usage;
        VkExtent2D imageExtent;

        bool copyToLocalDeviceMemory;


        size_t size;
        void *data;

        uint32_t threadIndex;
    };

    class i_Image {
    public:
        static void Init();

        static void Update();

        static void Cleanup();

        i_Image();

        i_Image(const i_ImageCreateInfo &createInfo);

        i_Image &operator=(const i_Image &other);

        i_Image(const i_Image &other);

        ~i_Image();


        static VkFormat GetSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                           VkFormatFeatureFlags features);

        static inline bool HasStencilComponent(VkFormat format);

        void TransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout) const;

        [[nodiscard]] VkImage GetImage() const;

        [[nodiscard]] VkImageView GetImageView() const;

        void Destruct();

    private:
        void CreateImage();

        void CreateImageView();

        void AllocateMemory();

        void CopyDataToDevice() const;

        static i_CommandBuffer GetFreeCommandBuffer(uint32_t threadIndex);

        static std::vector<std::vector<std::pair<i_CommandBuffer, bool> > > secondaryCommandBuffers;

        static void SubmitCommandBuffers();

        static void UpdateCleanup();

        static void CreateCommmandBuffers();

        static i_Fence commandBuffersFinishedExecutionFence;
        static std::set<uint32_t> commandPoolResetIndexes;
        static bool anyCommandBuffersRecorded;


        VkImage image;
        VkImageView imageView;
        VkDeviceMemory memory;

        std::shared_ptr<i_DataBuffer> stagingBuffer;

        i_Semaphore waitSemaphore;

        i_ImageCreateInfo createInfo{};

        static std::list<std::shared_ptr<i_DataBuffer> > bufferCleanupQueue;
        static std::vector<VkWriteDescriptorSet> writeDescriptorSetsQueue;


        std::shared_ptr<uint32_t> useCount;
    };
}

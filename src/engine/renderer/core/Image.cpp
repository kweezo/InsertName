#include "Image.hpp"

namespace renderer {
    constexpr uint32_t TARGET_SECONDARY_BUFFER_COUNT_PER_THREAD = 5;

    std::vector<std::vector<std::pair<i_CommandBuffer, bool> > > i_Image::secondaryCommandBuffers = {};
    i_Fence i_Image::commandBuffersFinishedExecutionFence = {};
    std::set<uint32_t> i_Image::commandPoolResetIndexes = {};
    std::list<std::shared_ptr<i_DataBuffer> > i_Image::bufferCleanupQueue = {};
    bool i_Image::anyCommandBuffersRecorded = false;
    std::vector<VkWriteDescriptorSet> i_Image::writeDescriptorSetsQueue = {};


    void i_Image::Init() {
        CreateCommmandBuffers();

        commandBuffersFinishedExecutionFence = i_Fence(false);
    }

    void i_Image::Update() {
        SubmitCommandBuffers();
        UpdateCleanup();
    }

    void i_Image::Cleanup() {
        secondaryCommandBuffers.clear();
        commandBuffersFinishedExecutionFence.~i_Fence();
    }

    VkFormat i_Image::GetSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                         VkFormatFeatureFlags features) {
        for (VkFormat format: candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(i_Device::GetPhysicalDevice(), format, &props);

            if ((props.linearTilingFeatures & features) == features) {
                if (tiling == VK_IMAGE_TILING_LINEAR) {
                    return format;
                }

                if (tiling == VK_IMAGE_TILING_OPTIMAL) {
                    return format;
                }
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    inline bool i_Image::HasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }


    void i_Image::SubmitCommandBuffers() {
        if (!anyCommandBuffersRecorded) {
            return;
        }


        VkFence finishedCopyingFenceRaw = commandBuffersFinishedExecutionFence.GetFence();

        std::vector<VkCommandBuffer> recordedCommandBuffers{};

        for (std::vector<std::pair<i_CommandBuffer, bool> > &commandBuffers: secondaryCommandBuffers) {
            for (std::pair<i_CommandBuffer, bool> &commandBuffer: commandBuffers) {
                if (!std::get<bool>(commandBuffer)) {
                    recordedCommandBuffers.push_back(std::get<i_CommandBuffer>(commandBuffer).GetCommandBuffer());
                }
            }
        }
        //TODO paralelize image and data buffer command buffer execution
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = recordedCommandBuffers.size();
        submitInfo.pCommandBuffers = recordedCommandBuffers.data();

        if (vkQueueSubmit(i_Device::GetTransferQueue(), 1, &submitInfo,
                          commandBuffersFinishedExecutionFence.GetFence()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit data buffer command buffer");
        }
        vkWaitForFences(i_Device::GetDevice(), 1, &finishedCopyingFenceRaw, VK_TRUE,
                        std::numeric_limits<uint64_t>::max());
        vkResetFences(i_Device::GetDevice(), 1, &finishedCopyingFenceRaw);
    }

    void i_Image::UpdateCleanup() {
        for (uint32_t i: commandPoolResetIndexes) {
            i_CommandBuffer::ResetPools(i_CommandBufferType::IMAGE, i);
        }
        anyCommandBuffersRecorded = false;

        bufferCleanupQueue.clear();
    }

    i_CommandBuffer i_Image::GetFreeCommandBuffer(uint32_t threadIndex) {
        anyCommandBuffersRecorded = true; // TODO CHECK ON THIS FUNCTION SHOULDNT BE WORKING

        for (std::pair<i_CommandBuffer, bool> &commandBuffer: secondaryCommandBuffers[threadIndex]) {
            if (std::get<bool>(commandBuffer)) {
                std::get<bool>(commandBuffer) = false;
                return std::get<i_CommandBuffer>(commandBuffer);
            }
        }


        i_CommandBufferCreateInfo secondaryCommandBufferInfo{};
        secondaryCommandBufferInfo.type = i_CommandBufferType::IMAGE;
        secondaryCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        secondaryCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER;
        secondaryCommandBufferInfo.threadIndex = threadIndex;

        secondaryCommandBuffers[threadIndex].emplace_back(i_CommandBuffer(secondaryCommandBufferInfo), false);
        return std::get<i_CommandBuffer>(secondaryCommandBuffers[threadIndex].back());
    }

    void i_Image::CreateCommmandBuffers() {
        uint32_t i = 0;
        secondaryCommandBuffers.resize(std::thread::hardware_concurrency());
        for (std::vector<std::pair<i_CommandBuffer, bool> > &commandBuffers: secondaryCommandBuffers) {
            i_CommandBufferCreateInfo stagingCommandBufferInfo{};
            stagingCommandBufferInfo.type = i_CommandBufferType::IMAGE;
            stagingCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            stagingCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER;
            stagingCommandBufferInfo.threadIndex = i;

            commandBuffers = std::vector<std::pair<i_CommandBuffer, bool> >(TARGET_SECONDARY_BUFFER_COUNT_PER_THREAD);

            for (std::pair<i_CommandBuffer, bool> &commandBuffer: commandBuffers) {
                std::get<bool>(commandBuffer) = true;
                std::get<i_CommandBuffer>(commandBuffer) = i_CommandBuffer(stagingCommandBufferInfo);
            }

            i = (i + 1) % std::thread::hardware_concurrency();
        }
    }

    i_Image::i_Image(): image(VK_NULL_HANDLE), imageView(VK_NULL_HANDLE), memory(VK_NULL_HANDLE), createInfo() {
    }

    i_Image::i_Image(const i_ImageCreateInfo &createInfo): image(VK_NULL_HANDLE), imageView(VK_NULL_HANDLE),
                                                           memory(VK_NULL_HANDLE), createInfo(createInfo) {
        CreateImage();
        AllocateMemory();

        vkBindImageMemory(i_Device::GetDevice(), image, memory, 0);

        CreateImageView();
        TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        useCount = std::make_shared<uint32_t>(1);

        if (createInfo.size == 0) {
            return;
        }

        if (i_Device::DeviceMemoryFree() && createInfo.copyToLocalDeviceMemory) {
            waitSemaphore = i_Semaphore(i_SemaphoreCreateInfo{});

            i_DataBufferCreateInfo bufferCreateInfo{};
            bufferCreateInfo.data = createInfo.data;
            bufferCreateInfo.size = createInfo.size;
            bufferCreateInfo.isDynamic = false;
            bufferCreateInfo.transferToLocalDeviceMemory = false;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            stagingBuffer = std::make_shared<i_DataBuffer>(i_DataBuffer(bufferCreateInfo));

            CopyDataToDevice();
        } else {
            i_DataBuffer::UploadDataToMemory(memory, createInfo.data, createInfo.size);
        }
    }

    void i_Image::CreateImage() {
        VkImageCreateInfo imageInfo{};

        i_QueueFamilyInfo queueFamilyInfo = i_Device::GetQueueFamilyInfo();
        if (queueFamilyInfo.transferFamilyFound) {
            imageInfo.queueFamilyIndexCount = 2;

            uint32_t queueFamilyIndices[] = {
                queueFamilyInfo.graphicsQueueCreateInfo.queueFamilyIndex,
                queueFamilyInfo.transferQueueCreateInfo.queueFamilyIndex
            };

            imageInfo.pQueueFamilyIndices = queueFamilyIndices;
            imageInfo.queueFamilyIndexCount = 2;
            imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        } else {
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        if (!i_Device::DeviceMemoryFree()) {
            createInfo.copyToLocalDeviceMemory = false;
        }

        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = createInfo.imageExtent.width;
        imageInfo.extent.height = createInfo.imageExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = createInfo.format;
        imageInfo.tiling = createInfo.copyToLocalDeviceMemory ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | createInfo.usage;
        if ((imageInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) ==
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        }
        imageInfo.initialLayout = createInfo.layout;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(i_Device::GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create an image");
        }
    }

    void i_Image::AllocateMemory() {
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(i_Device::GetDevice(), image, &memRequirements);

        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(i_Device::GetPhysicalDevice(), &memProperties);


        VkMemoryPropertyFlags memoryProperties = createInfo.copyToLocalDeviceMemory
                                                     ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                                                     : (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((memRequirements.memoryTypeBits & (1 << i)) && (
                    memProperties.memoryTypes[i].propertyFlags & memoryProperties)
                == memoryProperties) {
                VkMemoryAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex = i;

                if (vkAllocateMemory(i_Device::GetDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to allocate vertex buffer memory");
                }
                return;
            }
        }

        throw std::runtime_error("Failed to find a suitable memory type for VkImage");
    }

    void i_Image::CreateImageView() {
        VkImageViewCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = this->createInfo.format;
        createInfo.components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
        };
        createInfo.subresourceRange = {
            this->createInfo.aspectMask,
            0,
            1,
            0,
            1
        }; // TODO make mutable

        if (vkCreateImageView(i_Device::GetDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image view");
        }
    }

    void i_Image::CopyDataToDevice() const {
        i_CommandBuffer commandBuffer = GetFreeCommandBuffer(createInfo.threadIndex);

        VkCommandBufferInheritanceInfo inheritanceInfo{};
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

        VkBufferImageCopy copyRegion{};
        copyRegion.imageExtent = {createInfo.imageExtent.width, createInfo.imageExtent.height, 1};
        copyRegion.imageSubresource = {createInfo.aspectMask, 0, 0, 1};

        commandBuffer.BeginCommandBuffer(&inheritanceInfo, false);
        vkCmdCopyBufferToImage(commandBuffer.GetCommandBuffer(), stagingBuffer->GetBuffer(), image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
        commandBuffer.EndCommandBuffer();

        bufferCleanupQueue.push_front(stagingBuffer);
    }

    void i_Image::TransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout) const {
        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = oldLayout;
        imageMemoryBarrier.newLayout = newLayout;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange.aspectMask = createInfo.aspectMask;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // ?????

        VkCommandBufferInheritanceInfo inheritanceInfo = {};
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritanceInfo.renderPass = VK_NULL_HANDLE;
        inheritanceInfo.framebuffer = VK_NULL_HANDLE;
        inheritanceInfo.subpass = 0;
        inheritanceInfo.occlusionQueryEnable = VK_FALSE;
        inheritanceInfo.queryFlags = 0;
        inheritanceInfo.pipelineStatistics = 0;

        i_CommandBuffer commandBuffer = GetFreeCommandBuffer(createInfo.threadIndex);

        commandBuffer.BeginCommandBuffer(&inheritanceInfo, false);

        vkCmdPipelineBarrier(commandBuffer.GetCommandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        commandBuffer.EndCommandBuffer();
    }


    VkImage i_Image::GetImage() const {
        return image;
    }

    VkImageView i_Image::GetImageView() const {
        return imageView;
    }


    i_Image &i_Image::operator=(const i_Image &other) {
        if (this == &other) {
            return *this;
        }

        if (other.useCount == nullptr) {
            return *this;
        }

        Destruct();

        image = other.image;
        imageView = other.imageView;
        memory = other.memory;
        createInfo = other.createInfo;
        useCount = other.useCount;

        (*useCount)++;

        return *this;
    }

    i_Image::i_Image(const i_Image &other) {
        if (other.useCount == nullptr) {
            return;
        }

        image = other.image;
        imageView = other.imageView;
        memory = other.memory;
        createInfo = other.createInfo;
        useCount = other.useCount;

        (*useCount)++;
    }

    void i_Image::Destruct() {
        if (useCount == nullptr) {
            return;
        }

        if (*useCount == 1) {
            vkFreeMemory(i_Device::GetDevice(), memory, nullptr);
            vkDestroyImage(i_Device::GetDevice(), image, nullptr);
            vkDestroyImageView(i_Device::GetDevice(), imageView, nullptr);

            useCount.reset();
            return;
        }

        (*useCount)--;
    }

    i_Image::~i_Image() {
        Destruct();
    }
}

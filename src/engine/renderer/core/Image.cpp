#include "Image.hpp"

#define MAX_FREE_COMMAND_BUFFER_COUNT 3 

namespace renderer{
std::vector<ImageTransitionCMDInfo> ImageImpl::stagingBuffers = {};
Fence ImageImpl::finishedTransitioningFence = Fence();
CommandBuffer ImageImpl::primaryCommandBuffer = CommandBuffer();

ImageHandle Image::CreateImage(VkImageLayout layout, VkFormat format, VkImageAspectFlags aspectMask, VkImageUsageFlags usage,
     uint32_t width, uint32_t height, size_t size, void* data){
        return new ImageImpl(layout, format, aspectMask, usage, width, height, size, data);
}

void Image::Free(ImageHandle image){
    delete image;   
}

void Image::Initialize(){
    ImageImpl::Initialize();
}

void Image::UpdateCommandBuffers(){
    ImageImpl::UpdateCommandBuffers();
}

VkFormat Image::GetSupportedFormat(std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features){
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(Device::GetPhysicalDevice(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

inline bool Image::HasStencilComponent(VkFormat format){
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void ImageImpl::Initialize(){
    finishedTransitioningFence = Fence(0);
    CreateCommandBuffers();
}

ImageImpl::ImageImpl(){
    useCount = new uint32_t;
    (*useCount) = 1;
}

ImageImpl::ImageImpl(VkImageLayout layout, VkFormat format, VkImageAspectFlags aspectMask, VkImageUsageFlags usage,
 uint32_t width, uint32_t height, size_t size, void* data){


    CreateImage(width, height, format, usage);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(Device::GetDevice(), image, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(Device::GetPhysicalDevice(), &memProperties);

    VkMemoryPropertyFlagBits memoryProperties = Device::DeviceMemoryFree() ? (VkMemoryPropertyFlagBits)
    (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) : (VkMemoryPropertyFlagBits)
    (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++){
        if((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & memoryProperties)
         == memoryProperties){
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size ;
            allocInfo.memoryTypeIndex = i;

            if(vkAllocateMemory(Device::GetDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS){
                throw std::runtime_error("Failed to allocate vertex buffer memory");
            }

            break;
        }
    }

    vkBindImageMemory(Device::GetDevice(), image, memory, 0);

    CreateImageView(format, aspectMask);

    VkImageSubresourceLayers subresourceLayers = {};
    subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceLayers.baseArrayLayer = 0;
    subresourceLayers.layerCount = 1;
    subresourceLayers.mipLevel = 0;


    this->aspectMask = aspectMask;
    TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    loadDataInfo = {image, memory, size, data, {width, height, 1}, subresourceLayers, layout};


    useCount = new uint32_t;
    (*useCount) = 1;

}

void ImageImpl::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage){
    VkImageCreateInfo imageInfo = {};

    QueueFamilyInfo queueFamilyInfo = Device::GetQueueFamilyInfo();
    if(queueFamilyInfo.transferFamilyFound){
        imageInfo.queueFamilyIndexCount = 2;

        uint32_t queueFamilyIndices[] = {queueFamilyInfo.graphicsQueueCreateInfo.queueFamilyIndex,
         queueFamilyInfo.transferQueueCreateInfo.queueFamilyIndex};

        imageInfo.pQueueFamilyIndices = queueFamilyIndices;
        imageInfo.queueFamilyIndexCount = 2;
        imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    }else{
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if(vkCreateImage(Device::GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS){
        throw std::runtime_error("Failed to create image");
    }
}


void ImageImpl::CreateImageView(VkFormat format, VkImageAspectFlags aspectMask){
    VkImageViewCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components = {
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
    };
    createInfo.subresourceRange = {
        aspectMask,
        0,
        1,
        0,
        1
    };// make mutable

    if(vkCreateImageView(Device::GetDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS){
        throw std::runtime_error("Failed to create image view");
    }
}

VkImageView ImageImpl::GetImageView(){
    return imageView;
}


void ImageImpl::TransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout){
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.oldLayout = oldLayout;
    imageMemoryBarrier.newLayout = newLayout;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
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

    CommandBuffer commandBuffer = GetFreeCommandBuffer(this);
    commandBuffer.BeginCommandBuffer(&inheritanceInfo); 
    vkCmdPipelineBarrier(commandBuffer.GetCommandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    commandBuffer.EndCommandBuffer();
}

void ImageImpl::LoadDataIntoImage(){
    DataBuffer::LoadDataIntoImage(loadDataInfo.image, loadDataInfo.size, loadDataInfo.data, loadDataInfo.extent, loadDataInfo.subresource, loadDataInfo.layout);
}

void ImageImpl::UpdateCommandBuffers(){
    std::vector<VkCommandBuffer> commandBuffers;
    for(int i = 0; i < stagingBuffers.size(); i++){
        if(!stagingBuffers[i].free){
            commandBuffers.push_back(stagingBuffers[i].commandBuffer.GetCommandBuffer());
        }
    }
    
    if(commandBuffers.empty()){
        return;
    }

    VkFence fence = finishedTransitioningFence.GetFence();

    vkResetFences(Device::GetDevice(), 1, &fence);

    primaryCommandBuffer.BeginCommandBuffer(nullptr);
    vkCmdExecuteCommands(primaryCommandBuffer.GetCommandBuffer(), commandBuffers.size(), commandBuffers.data());
    primaryCommandBuffer.EndCommandBuffer();



    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    VkCommandBuffer commandBufferHandle = primaryCommandBuffer.GetCommandBuffer();
    submitInfo.pCommandBuffers = &commandBufferHandle;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.signalSemaphoreCount = 0;

    if(vkQueueSubmit(Device::GetGraphicsQueue(), 1, &submitInfo, finishedTransitioningFence.GetFence()) != VK_SUCCESS){
        throw std::runtime_error("Failed to submit transfer command buffer");
    }
    
    vkWaitForFences(Device::GetDevice(), 1, &fence, VK_TRUE, UINT64_MAX);

    for(ImageCopyCMDInfo& buffer : stagingBuffers){
        buffer.commandBuffer.ResetCommandBuffer();
        if(buffer.free){
            continue;
        }
        buffer.free = true;
    }

    while(stagingBuffers.size() > MAX_FREE_COMMAND_BUFFER_COUNT){
        stagingBuffers.erase(stagingBuffers.begin());
    }
}


void ImageImpl::CreateCommandBuffers(){
    for(int i = 0; i < MAX_FREE_COMMAND_BUFFER_COUNT; i++){
        stagingBuffers.push_back({CommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG | COMMAND_BUFFER_GRAPHICS_FLAG/*cuz of stencil formats and shit*/, 0), true});
    }
    primaryCommandBuffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, COMMAND_BUFFER_GRAPHICS_FLAG, 0);
}

VkImage ImageImpl::GetImage(){
    return image;

}

CommandBuffer ImageImpl::GetFreeCommandBuffer(ImageHandle image){
    for(ImageCopyCMDInfo& buffer : stagingBuffers){
        if(buffer.free){
            buffer.free = false;
            buffer.image = image;
            return buffer.commandBuffer;
        }
    }

    stagingBuffers.push_back({CommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY,
     COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG | COMMAND_BUFFER_TRANSFER_FLAG, 0), false}); //TODO implement multithreading¸

    return stagingBuffers.back().commandBuffer;
}

ImageImpl::ImageImpl(const ImageImpl& other){
    loadDataInfo = other.loadDataInfo;
    image = other.image;
    memory = other.memory;
    useCount = other.useCount;
    imageView = other.imageView;
    aspectMask = other.aspectMask;
    (*useCount)++;
}

ImageImpl& ImageImpl::operator=(const ImageImpl& other){
    if(this == &other){
        return *this;
    }

    loadDataInfo = other.loadDataInfo;
    image = other.image;
    memory = other.memory;
    useCount = other.useCount;
    imageView = other.imageView;
    aspectMask = other.aspectMask;
    (*useCount)++;

    return *this;
}

ImageImpl::~ImageImpl(){
    if((*useCount) <= 1){
        vkDestroyImageView(Device::GetDevice(), imageView, nullptr);
        vkDestroyImage(Device::GetDevice(), image, nullptr);
        vkFreeMemory(Device::GetDevice(), memory, nullptr);
    }
}

void ImageImpl::Cleanup(){
    finishedTransitioningFence.~Fence();
    primaryCommandBuffer.~CommandBuffer();
    for (ImageTransitionCMDInfo& buffer : stagingBuffers){
        buffer.commandBuffer.~CommandBuffer();
    }
}

}



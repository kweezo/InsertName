#include "Image.hpp"

#define MAX_FREE_COMMAND_BUFFER_COUNT 3 

namespace renderer{
std::vector<ImageTransitionCMDInfo> ImageImpl::stagingBuffers = {};
Fence ImageImpl::finishedTransitioningFence = Fence();
CommandBuffer ImageImpl::primaryCommandBuffer = CommandBuffer();

ImageHandle Image::CreateImage(VkImageLayout layout, VkFormat format, uint32_t width,
     uint32_t height, size_t size, void* data){
        return new ImageImpl(layout, format, width, height, size, data);
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

void ImageImpl::Initialize(){
    finishedTransitioningFence = Fence(0);
    CreateCommandBuffers();
}

ImageImpl::ImageImpl(){
    useCount = new uint32_t;
    (*useCount) = 1;
}

ImageImpl::ImageImpl(VkImageLayout layout, VkFormat format, uint32_t width,
     uint32_t height, size_t size, void* data){

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
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if(vkCreateImage(Device::GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS){
        throw std::runtime_error("Failed to create image");
    }

    

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(Device::GetDevice(), image, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(Device::GetPhysicalDevice(), &memProperties);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++){
        if((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
         == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT){
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

    VkImageSubresourceLayers subresourceLayers = {};
    subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceLayers.baseArrayLayer = 0;
    subresourceLayers.layerCount = 1;
    subresourceLayers.mipLevel = 0;

    CommandBuffer commandBuffer = GetFreeCommandBuffer(this);

    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;//make this a parameter
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

    commandBuffer.BeginCommandBuffer(0, &inheritanceInfo); 
    vkCmdPipelineBarrier(commandBuffer.GetCommandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    commandBuffer.EndCommandBuffer();


    loadDataInfo = {image, memory, size, data, {width, height, 1}, subresourceLayers};

    useCount = new uint32_t;
    (*useCount) = 1;

}

void ImageImpl::TransitionImageLayout(){
    DataBuffer::LoadDataIntoImage(loadDataInfo.image, loadDataInfo.size, loadDataInfo.data, loadDataInfo.extent, loadDataInfo.subresource);
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

    primaryCommandBuffer.BeginCommandBuffer(0, nullptr);
    vkCmdExecuteCommands(primaryCommandBuffer.GetCommandBuffer(), commandBuffers.size(), commandBuffers.data());
    primaryCommandBuffer.EndCommandBuffer();



    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    VkCommandBuffer commandBufferHandle = primaryCommandBuffer.GetCommandBuffer();
    submitInfo.pCommandBuffers = &commandBufferHandle;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.signalSemaphoreCount = 0;

    if(vkQueueSubmit(Device::GetTransferQueue(), 1, &submitInfo, finishedTransitioningFence.GetFence()) != VK_SUCCESS){
        throw std::runtime_error("Failed to submit transfer command buffer");
    }
    
    vkWaitForFences(Device::GetDevice(), 1, &fence, VK_TRUE, UINT64_MAX);

    for(ImageCopyCMDInfo& buffer : stagingBuffers){
        if(buffer.free){
            continue;
        }
        buffer.image->TransitionImageLayout();
        buffer.free = true;
    }

    while(stagingBuffers.size() > MAX_FREE_COMMAND_BUFFER_COUNT){
        stagingBuffers.erase(stagingBuffers.begin());
    }
}


void ImageImpl::CreateCommandBuffers(){
    for(int i = 0; i < MAX_FREE_COMMAND_BUFFER_COUNT; i++){
        stagingBuffers.push_back({CommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG | COMMAND_BUFFER_TRANSFER_FLAG, nullptr), true});
    }
    primaryCommandBuffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, COMMAND_BUFFER_TRANSFER_FLAG, nullptr);
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
     COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG | COMMAND_BUFFER_TRANSFER_FLAG, nullptr), false});

    return stagingBuffers.back().commandBuffer;
}

VkImage ImageImpl::GetImage(){
    return image;

}

ImageImpl::ImageImpl(const ImageImpl& other){
    loadDataInfo = other.loadDataInfo;
    image = other.image;
    memory = other.memory;
    useCount = other.useCount;
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
    (*useCount)++;

    return *this;
}

ImageImpl::~ImageImpl(){
    if((*useCount) <= 1){
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



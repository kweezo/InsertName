#include "Image.hpp"

namespace renderer{

const uint32_t TARGET_SECONDARY_BUFFER_COUNT_PER_THREAD = 5;
    
std::vector<std::vector<std::pair<__CommandBuffer, bool>>> __Image::secondaryCommandBuffers = {};
__CommandBuffer __Image::primaryCommandBuffer = {};
__Fence __Image::finishedPrimaryCommandBufferExecutionFence = {};
std::set<uint32_t> __Image::commandPoolResetIndexes = {};
std::list<std::pair<VkBuffer, VkDeviceMemory>> __Image::bufferAndMemoryCleanupQueue = { };
bool __Image::primaryCommandBufferRecorded = false;


void __Image::Init(){
    CreateCommmandBuffers();

    finishedPrimaryCommandBufferExecutionFence = __Fence(false);
}

void __Image::Update(){
    RecordPrimaryCommandBuffer();
    SubmitPrimaryCommandBuffer();
    UpdateCleanup();
}

void __Image::Cleanup(){
    secondaryCommandBuffers.clear();
    primaryCommandBuffer.~__CommandBuffer();
    finishedPrimaryCommandBufferExecutionFence.~__Fence();
}
void __Image::RecordPrimaryCommandBuffer(){
    
    std::vector<VkCommandBuffer> usedSecondaryCommandBuffers;

    uint32_t i = 0;
    for(std::vector<std::pair<__CommandBuffer, bool>>& commandBuffers : secondaryCommandBuffers){
        for(std::pair<__CommandBuffer, bool>& commandBuffer : commandBuffers){
            if(!std::get<bool>(commandBuffer)){
                usedSecondaryCommandBuffers.push_back(std::get<__CommandBuffer>(commandBuffer).GetCommandBuffer());
                std::get<bool>(commandBuffer) = true;

                commandPoolResetIndexes.emplace(i);
            }
        }
        i++;
    }

    if(usedSecondaryCommandBuffers.size() == 0){
        return;
    }

    primaryCommandBuffer.BeginCommandBuffer(nullptr, false);
    vkCmdExecuteCommands(primaryCommandBuffer.GetCommandBuffer(), usedSecondaryCommandBuffers.size(), usedSecondaryCommandBuffers.data());
    primaryCommandBuffer.EndCommandBuffer();
    
    primaryCommandBufferRecorded = true;
}

VkFormat __Image::GetSupportedFormat(std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features){
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(__Device::GetPhysicalDevice(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

inline bool __Image::HasStencilComponent(VkFormat format){
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


void __Image::SubmitPrimaryCommandBuffer(){
    if(!primaryCommandBufferRecorded){
        return;
    }

    VkCommandBuffer primaryCommandBufferRaw = primaryCommandBuffer.GetCommandBuffer();
    VkFence finishedCopyingFenceRaw = finishedPrimaryCommandBufferExecutionFence.GetFence();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &primaryCommandBufferRaw;

    if(vkQueueSubmit(__Device::GetTransferQueue(), 1, &submitInfo, finishedPrimaryCommandBufferExecutionFence.GetFence()) != VK_SUCCESS){
        throw std::runtime_error("Failed to submit data buffer command buffer");
    }

    vkWaitForFences(__Device::GetDevice(), 1, &finishedCopyingFenceRaw, VK_TRUE, std::numeric_limits<uint64_t>::max());
}

void __Image::UpdateCleanup(){
    for(uint32_t i : commandPoolResetIndexes){
        __CommandBuffer::ResetPools(__CommandBufferType::IMAGE, i); 
    }
    primaryCommandBufferRecorded = false;

    for(std::pair<VkBuffer, VkDeviceMemory>& bufferAndMemory : bufferAndMemoryCleanupQueue){
        vkFreeMemory(__Device::GetDevice(), std::get<1>(bufferAndMemory), nullptr);
        vkDestroyBuffer(__Device::GetDevice(), std::get<0>(bufferAndMemory), nullptr);
    }

}

__CommandBuffer __Image::GetFreeCommandBuffer(uint32_t threadIndex){
    for(std::pair<__CommandBuffer, bool>& commandBuffer : secondaryCommandBuffers[threadIndex]){
        if(std::get<bool>(commandBuffer)){
            std::get<bool>(commandBuffer) = false;
            return std::get<__CommandBuffer>(commandBuffer);
        }
    }

    __CommandBufferCreateInfo secondaryCommandBufferInfo;
    secondaryCommandBufferInfo.type = __CommandBufferType::DATA;
    secondaryCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    secondaryCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
    secondaryCommandBufferInfo.threadIndex = threadIndex;

    secondaryCommandBuffers[threadIndex].push_back({__CommandBuffer(secondaryCommandBufferInfo), false});
    return std::get<__CommandBuffer>(secondaryCommandBuffers[threadIndex].back());
}

void __Image::CreateCommmandBuffers(){
    __CommandBufferCreateInfo commandBufferInfo{};
    commandBufferInfo.type = __CommandBufferType::GENERIC;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
    commandBufferInfo.threadIndex = 0;

    primaryCommandBuffer = __CommandBuffer(commandBufferInfo);

    uint32_t i = 0;
    secondaryCommandBuffers.resize(std::thread::hardware_concurrency());
    for(std::vector<std::pair<__CommandBuffer, bool>>& commandBuffers : secondaryCommandBuffers){

        __CommandBufferCreateInfo stagingCommandBufferInfo;
        stagingCommandBufferInfo.type = __CommandBufferType::DATA;
        stagingCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        stagingCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
        stagingCommandBufferInfo.threadIndex = i;

        commandBuffers = std::vector<std::pair<__CommandBuffer, bool>>(TARGET_SECONDARY_BUFFER_COUNT_PER_THREAD);

        for(std::pair<__CommandBuffer, bool>& commandBuffer : commandBuffers){
            std::get<bool>(commandBuffer) = true;
            std::get<__CommandBuffer>(commandBuffer) = __CommandBuffer(stagingCommandBufferInfo);
        }

        i = (i + 1) % std::thread::hardware_concurrency();
    }
}

__Image::__Image(){
    useCount = std::make_shared<uint32_t>(1);
}

__Image::__Image(__ImageCreateInfo createInfo): createInfo(createInfo) {
    CreateImage();
    AllocateMemory();

    vkBindImageMemory(__Device::GetDevice(), image, memory, 0);

    CreateImageView();
    TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    useCount = std::make_shared<uint32_t>(1);

    if(createInfo.size == 0){
        return;
    }

    if(__Device::DeviceMemoryFree() && createInfo.copyToLocalDeviceMemory){
        __DataBuffer::CreateBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, createInfo.size);

        __DataBuffer::AllocateMemory(stagingMemory, stagingBuffer, createInfo.size, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        __DataBuffer::UploadDataToMemory(stagingMemory, createInfo.data, createInfo.size);

        CopyDataToDevice();
    }else{
        __DataBuffer::UploadDataToMemory(memory, createInfo.data, createInfo.size);
    }
}

void __Image::CreateImage(){
    VkImageCreateInfo imageInfo{};

    __QueueFamilyInfo queueFamilyInfo = __Device::GetQueueFamilyInfo();
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
    imageInfo.extent.width = createInfo.imageExtent.width;
    imageInfo.extent.height = createInfo.imageExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = createInfo.format;
    imageInfo.tiling = createInfo.copyToLocalDeviceMemory ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR;
    imageInfo.initialLayout = createInfo.layout;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | createInfo.usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if(vkCreateImage(__Device::GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS){
        throw std::runtime_error("Failed to create an image");
    }
}

void __Image::AllocateMemory(){
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(__Device::GetDevice(), image, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(__Device::GetPhysicalDevice(), &memProperties);

    VkMemoryPropertyFlags memoryProperties = createInfo.copyToLocalDeviceMemory ? 
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT :
     (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++){
        if((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & memoryProperties)
         == memoryProperties){
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = i;

            if(vkAllocateMemory(__Device::GetDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS){
                throw std::runtime_error("Failed to allocate vertex buffer memory");
            }
            return;
        }
    }

    throw std::runtime_error("Failed to find a suitable memory type for VkImage");
}

void __Image::CreateImageView(){
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
    };// TODO make mutable

    if(vkCreateImageView(__Device::GetDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS){
        throw std::runtime_error("Failed to create image view");
    }
}

void __Image::CopyDataToDevice(){
    __CommandBuffer commandBuffer = GetFreeCommandBuffer(createInfo.threadIndex);

    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

    VkBufferImageCopy copyRegion{};
    copyRegion.imageExtent = {createInfo.imageExtent.width, createInfo.imageExtent.height, 1};
    copyRegion.imageSubresource = {createInfo.aspectMask, 0, 0, 1};

    commandBuffer.BeginCommandBuffer(&inheritanceInfo, false);
    vkCmdCopyBufferToImage(commandBuffer.GetCommandBuffer(), stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
    commandBuffer.EndCommandBuffer();

    bufferAndMemoryCleanupQueue.push_front({stagingBuffer, stagingMemory});
}

void __Image::TransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout){

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

    __CommandBuffer commandBuffer = GetFreeCommandBuffer(createInfo.threadIndex);

    commandBuffer.BeginCommandBuffer(&inheritanceInfo, false); 


    vkCmdPipelineBarrier(commandBuffer.GetCommandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

    commandBuffer.EndCommandBuffer();
}

VkImage __Image::GetImage(){
    return image;
}

VkImageView __Image::GetImageView(){
    return imageView;
}


__Image __Image::operator=(const __Image& other){
    if(this == &other){
        return *this;
    }

    image = other.image;
    imageView = other.imageView;
    memory = other.memory;
    createInfo = other.createInfo;
    useCount = other.useCount;

    (*useCount.get())++;

    return *this;
}

__Image::__Image(const __Image& other){
    image = other.image;
    imageView = other.imageView;
    memory = other.memory;
    createInfo = other.createInfo;
    useCount = other.useCount;

    (*useCount.get())++;
}

__Image::~__Image(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount.get() == 1){
        vkFreeMemory(__Device::GetDevice(), memory, nullptr);
        vkDestroyImage(__Device::GetDevice(), image, nullptr);
        vkDestroyImageView(__Device::GetDevice(), imageView, nullptr);

        useCount.reset();
        return;
    }

    (*useCount.get())--;
}

}
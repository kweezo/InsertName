#include "Image.hpp"

namespace renderer{

const uint32_t TARGET_SECONDARY_BUFFER_COUNT_PER_THREAD = 5;
    
std::vector<std::vector<std::pair<CommandBuffer, bool>>> Image::secondaryCommandBuffers = {};
CommandBuffer Image::primaryCommandBuffer = {};


void Image::Init(){
    CreateCommmandBuffers();

    finishedPrimaryCommandBufferExecutionFence = Fence(false);
}

void Image::Update(){
    RecordPrimaryCommandBuffer();
    SubmitPrimaryCommandBuffer();
    UpdateCleanup();
}

void Image::Cleanup(){
    secondaryCommandBuffers.clear();
    primaryCommandBuffer.~CommandBuffer();
}

void Image::RecordPrimaryCommandBuffer(){
    
    std::vector<VkCommandBuffer> usedSecondaryCommandBuffers;

    for(std::vector<std::pair<CommandBuffer, bool>>& commandBuffers : secondaryCommandBuffers){
        for(std::pair<CommandBuffer, bool>& commandBuffer : commandBuffers){
            if(!std::get<bool>(commandBuffer)){
                usedSecondaryCommandBuffers.push_back(std::get<CommandBuffer>(commandBuffer).GetCommandBuffer());
            }
        }
    }

    primaryCommandBuffer.BeginCommandBuffer(nullptr, false);
    vkCmdExecuteCommands(primaryCommandBuffer.GetCommandBuffer(), usedSecondaryCommandBuffers.size(), usedSecondaryCommandBuffers.data());
    primaryCommandBuffer.EndCommandBuffer();

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


void Image::SubmitPrimaryCommandBuffer(){
    VkCommandBuffer primaryCommandBufferRaw = primaryCommandBuffer.GetCommandBuffer();
    VkFence finishedCopyingFenceRaw = finishedPrimaryCommandBufferExecutionFence.GetFence();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &primaryCommandBufferRaw;

    if(vkQueueSubmit(Device::GetTransferQueue(), 1, &submitInfo, finishedPrimaryCommandBufferExecutionFence.GetFence()) != VK_SUCCESS){
        throw std::runtime_error("Failed to submit data buffer command buffer");
    }

    vkWaitForFences(Device::GetDevice(), 1, &finishedCopyingFenceRaw, VK_TRUE, std::numeric_limits<uint64_t>::max());
}

void Image::UpdateCleanup(){
    CommandBuffer::ResetPools(CommandBufferType::IMAGE); 
}

CommandBuffer Image::GetFreeCommandBuffer(uint32_t threadIndex){
    for(std::pair<CommandBuffer, bool>& commandBuffer : secondaryCommandBuffers[threadIndex]){
        if(std::get<bool>(commandBuffer)){
            std::get<bool>(commandBuffer) = false;
            return std::get<CommandBuffer>(commandBuffer);
        }
    }

    CommandBufferCreateInfo secondaryCommandBufferInfo;
    secondaryCommandBufferInfo.type = CommandBufferType::DATA;
    secondaryCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    secondaryCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
    secondaryCommandBufferInfo.threadIndex = threadIndex;

    secondaryCommandBuffers[threadIndex].push_back({CommandBuffer(secondaryCommandBufferInfo), false});
    return std::get<CommandBuffer>(secondaryCommandBuffers[threadIndex].back());
}

void Image::CreateCommmandBuffers(){
    CommandBufferCreateInfo commandBufferInfo{};
    commandBufferInfo.type = CommandBufferType::GENERIC;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
    commandBufferInfo.threadIndex = 0;

    primaryCommandBuffer = CommandBuffer(commandBufferInfo);

    uint32_t i = 0;
    secondaryCommandBuffers.resize(std::thread::hardware_concurrency());
    for(std::vector<std::pair<CommandBuffer, bool>>& commandBuffers : secondaryCommandBuffers){

        CommandBufferCreateInfo stagingCommandBufferInfo;
        stagingCommandBufferInfo.type = CommandBufferType::DATA;
        stagingCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        stagingCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
        stagingCommandBufferInfo.threadIndex = i;

        commandBuffers = std::vector<std::pair<CommandBuffer, bool>>(TARGET_SECONDARY_BUFFER_COUNT_PER_THREAD, {CommandBuffer(stagingCommandBufferInfo), false});

        i = (i + 1) % std::thread::hardware_concurrency();
    }
}

Image::Image(){
    useCount = std::make_shared<uint32_t>(1);
}

Image::Image(ImageCreateInfo createInfo): createInfo(createInfo) {
    CreateImage();
    AllocateMemory();
    TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    useCount = std::make_shared<uint32_t>(1);
}

void Image::CreateImage(){
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
    imageInfo.extent.width = createInfo.imageExtent.width;
    imageInfo.extent.height = createInfo.imageExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = createInfo.format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | createInfo.usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if(vkCreateImage(Device::GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS){
        throw std::runtime_error("Failed to create an image");
    }
}

void Image::AllocateMemory(){
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
}

void Image::CreateImageView(){
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

    if(vkCreateImageView(Device::GetDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS){
        throw std::runtime_error("Failed to create image view");
    }
}

void Image::TransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout){
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

    CommandBuffer commandBuffer = GetFreeCommandBuffer(createInfo.threadIndex);

    commandBuffer.BeginCommandBuffer(&inheritanceInfo, false); 

    vkCmdPipelineBarrier(commandBuffer.GetCommandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

    commandBuffer.EndCommandBuffer();
}

VkImage Image::GetImage(){
    return image;
}

VkImageView Image::GetImageView(){
    return imageView;
}


Image Image::operator=(const Image& other){
    if(this == &other){
        return *this;
    }

    image = other.image;
    imageView = other.imageView;
    memory = other.memory;
    createInfo = other.createInfo;
    useCount = other.useCount;

    (*useCount.get())++;
}

Image::Image(const Image& other){
    image = other.image;
    imageView = other.imageView;
    memory = other.memory;
    createInfo = other.createInfo;
    useCount = other.useCount;

    (*useCount.get())++;
}

Image::~Image(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount.get() == 1){
        vkFreeMemory(Device::GetDevice(), memory, nullptr);
        vkDestroyImage(Device::GetDevice(), image, nullptr);
        vkDestroyImageView(Device::GetDevice(), imageView, nullptr);

        useCount.reset();
        return;
    }

    (*useCount.get())--;
}

}
#include "Image.hpp"

namespace renderer{

const uint32_t TARGET_SECONDARY_BUFFER_COUNT_PER_THREAD = 5;
    
std::vector<std::vector<std::pair<_CommandBuffer, bool>>> _Image::secondaryCommandBuffers = {};
_Fence _Image::commandBuffersFinishedExecutionFence ={};
std::set<uint32_t> _Image::commandPoolResetIndexes = {};
std::list<std::shared_ptr<_DataBuffer>> _Image::bufferCleanupQueue = { };
bool _Image::anyCommandBuffersRecorded = false;
std::vector<VkWriteDescriptorSet> _Image::writeDescriptorSetsQueue = {};


void _Image::Init(){
    CreateCommmandBuffers();

    commandBuffersFinishedExecutionFence = _Fence(false);
}

void _Image::Update(){
    SubmitCommandBuffers();
    UpdateCleanup();
}

void _Image::Cleanup(){
    secondaryCommandBuffers.clear();
    commandBuffersFinishedExecutionFence.~_Fence();
}

VkFormat _Image::GetSupportedFormat(std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features){
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_Device::GetPhysicalDevice(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

inline bool _Image::HasStencilComponent(VkFormat format){
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


void _Image::SubmitCommandBuffers(){
    if(!anyCommandBuffersRecorded){
        return;
    }


    VkFence finishedCopyingFenceRaw = commandBuffersFinishedExecutionFence.GetFence();

    std::vector<VkCommandBuffer> recordedCommandBuffers{};

    for(std::vector<std::pair<_CommandBuffer, bool>>& commandBuffers : secondaryCommandBuffers){
        for(std::pair<_CommandBuffer, bool>& commandBuffer : commandBuffers){
            if(!std::get<bool>(commandBuffer)){
                recordedCommandBuffers.push_back(std::get<_CommandBuffer>(commandBuffer).GetCommandBuffer());
            }
        }
    }
//TODO paralelize image and data buffer command buffer execution
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = recordedCommandBuffers.size();
    submitInfo.pCommandBuffers = recordedCommandBuffers.data();

    if(vkQueueSubmit(_Device::GetTransferQueue(), 1, &submitInfo, commandBuffersFinishedExecutionFence.GetFence()) != VK_SUCCESS){
        throw std::runtime_error("Failed to submit data buffer command buffer");
    }
    vkWaitForFences(_Device::GetDevice(), 1, &finishedCopyingFenceRaw, VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(_Device::GetDevice(), 1, &finishedCopyingFenceRaw);
    
}

void _Image::UpdateCleanup(){
    for(uint32_t i : commandPoolResetIndexes){
        _CommandBuffer::ResetPools(_CommandBufferType::IMAGE, i); 
    }
    anyCommandBuffersRecorded = false;

    bufferCleanupQueue.clear();
}

_CommandBuffer _Image::GetFreeCommandBuffer(uint32_t threadIndex){
    anyCommandBuffersRecorded = true; // TODO CHECK ON THIS FUNCTION SHOULDNT BE WORKING

    for(std::pair<_CommandBuffer, bool>& commandBuffer : secondaryCommandBuffers[threadIndex]){
        if(std::get<bool>(commandBuffer)){
            std::get<bool>(commandBuffer) = false;
            return std::get<_CommandBuffer>(commandBuffer);
        }
    }


    _CommandBufferCreateInfo secondaryCommandBufferInfo;
    secondaryCommandBufferInfo.type = _CommandBufferType::IMAGE;
    secondaryCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    secondaryCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER;
    secondaryCommandBufferInfo.threadIndex = threadIndex;

    secondaryCommandBuffers[threadIndex].push_back({_CommandBuffer(secondaryCommandBufferInfo), false});
    return std::get<_CommandBuffer>(secondaryCommandBuffers[threadIndex].back());
}

void _Image::CreateCommmandBuffers(){
    uint32_t i = 0;
    secondaryCommandBuffers.resize(std::thread::hardware_concurrency());
    for(std::vector<std::pair<_CommandBuffer, bool>>& commandBuffers : secondaryCommandBuffers){

        _CommandBufferCreateInfo stagingCommandBufferInfo;
        stagingCommandBufferInfo.type = _CommandBufferType::IMAGE;
        stagingCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        stagingCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER; 
        stagingCommandBufferInfo.threadIndex = i;

        commandBuffers = std::vector<std::pair<_CommandBuffer, bool>>(TARGET_SECONDARY_BUFFER_COUNT_PER_THREAD);

        for(std::pair<_CommandBuffer, bool>& commandBuffer : commandBuffers){
            std::get<bool>(commandBuffer) = true;
            std::get<_CommandBuffer>(commandBuffer) = _CommandBuffer(stagingCommandBufferInfo);
        }

        i = (i + 1) % std::thread::hardware_concurrency();
    }
}

_Image::_Image(){
}

_Image::_Image(_ImageCreateInfo createInfo): createInfo(createInfo) {
    CreateImage();
    AllocateMemory();

    vkBindImageMemory(_Device::GetDevice(), image, memory, 0);

    CreateImageView();
    TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    useCount = std::make_shared<uint32_t>(1);

    if(createInfo.size == 0){
        return;
    }

    if(_Device::DeviceMemoryFree() && createInfo.copyToLocalDeviceMemory){
        
        waitSemaphore = _Semaphore(_SemaphoreCreateInfo{});

        _DataBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.data = createInfo.data;
        bufferCreateInfo.size = createInfo.size;
        bufferCreateInfo.isDynamic = false;
        bufferCreateInfo.transferToLocalDeviceMemory = false;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        stagingBuffer = std::make_shared<_DataBuffer>(_DataBuffer(bufferCreateInfo));

        CopyDataToDevice();
    }else{
        _DataBuffer::UploadDataToMemory(memory, createInfo.data, createInfo.size);
    }
}

void _Image::CreateImage(){
    VkImageCreateInfo imageInfo{};

    _QueueFamilyInfo queueFamilyInfo = _Device::GetQueueFamilyInfo();
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

    if(!_Device::DeviceMemoryFree()){
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
    if((imageInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT){
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    }
    imageInfo.initialLayout = createInfo.layout;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if(vkCreateImage(_Device::GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS){
        throw std::runtime_error("Failed to create an image");
    }
}

void _Image::AllocateMemory(){
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_Device::GetDevice(), image, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_Device::GetPhysicalDevice(), &memProperties);


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

            if(vkAllocateMemory(_Device::GetDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS){
                throw std::runtime_error("Failed to allocate vertex buffer memory");
            }
            return;
        }
    }

    throw std::runtime_error("Failed to find a suitable memory type for VkImage");
}

void _Image::CreateImageView(){
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

    if(vkCreateImageView(_Device::GetDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS){
        throw std::runtime_error("Failed to create image view");
    }
}

void _Image::CopyDataToDevice(){

    _CommandBuffer commandBuffer = GetFreeCommandBuffer(createInfo.threadIndex);

    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

    VkBufferImageCopy copyRegion{};
    copyRegion.imageExtent = {createInfo.imageExtent.width, createInfo.imageExtent.height, 1};
    copyRegion.imageSubresource = {createInfo.aspectMask, 0, 0, 1};

    commandBuffer.BeginCommandBuffer(&inheritanceInfo, false);
    vkCmdCopyBufferToImage(commandBuffer.GetCommandBuffer(), stagingBuffer->GetBuffer(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
    commandBuffer.EndCommandBuffer();

    bufferCleanupQueue.push_front(stagingBuffer);
}

void _Image::TransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout){

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

    _CommandBuffer commandBuffer = GetFreeCommandBuffer(createInfo.threadIndex);

    commandBuffer.BeginCommandBuffer(&inheritanceInfo, false); 

    vkCmdPipelineBarrier(commandBuffer.GetCommandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

    commandBuffer.EndCommandBuffer();
}


VkImage _Image::GetImage(){
    return image;
}

VkImageView _Image::GetImageView(){
    return imageView;
}


_Image _Image::operator=(const _Image& other){
    if(this == &other){
        return *this;
    }

    if(other.useCount.get() == nullptr){
        return *this;
    }

    Destruct();

    image = other.image;
    imageView = other.imageView;
    memory = other.memory;
    createInfo = other.createInfo;
    useCount = other.useCount;

    (*useCount.get())++;

    return *this;
}

_Image::_Image(const _Image& other){
    if(other.useCount.get() == nullptr){
        return;
    }

    image = other.image;
    imageView = other.imageView;
    memory = other.memory;
    createInfo = other.createInfo;
    useCount = other.useCount;

    (*useCount.get())++;
}

void _Image::Destruct(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount.get() == 1){
        vkFreeMemory(_Device::GetDevice(), memory, nullptr);
        vkDestroyImage(_Device::GetDevice(), image, nullptr);
        vkDestroyImageView(_Device::GetDevice(), imageView, nullptr);

        useCount.reset();
        return;
    }

    (*useCount.get())--;
}

_Image::~_Image(){
    Destruct();
}

}
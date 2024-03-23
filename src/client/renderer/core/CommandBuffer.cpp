#include "CommandBuffer.hpp"

CommandBuffer::CommandBuffer(): commandBuffer(VK_NULL_HANDLE) {
    useCount = new uint32_t;
    useCount[0] = 1;
}

CommandBuffer::CommandBuffer(VkCommandBufferLevel level, uint32_t flags, GraphicsPipeline* pipeline){
    if(flags & COMMAND_BUFFER_GRAPHICS_FLAG == COMMAND_BUFFER_GRAPHICS_FLAG && pipeline == nullptr){
        throw std::runtime_error("Graphics pipeline must be provided to the graphics command buffer");
    }
    
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = (flags & COMMAND_BUFFER_GRAPHICS_FLAG == COMMAND_BUFFER_GRAPHICS_FLAG)
     ? CommandPool::GetGraphicsCommandPool() : CommandPool::GetTransferCommandPool();
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    if(vkAllocateCommandBuffers(Device::GetDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate command buffers");
    }

    useCount = new uint32_t;
    useCount[0] = 1;

    this->flags = flags;
    this->pipeline = pipeline;
    this->level = level;
}

void CommandBuffer::BeginCommandBuffer(uint32_t imageIndex, VkCommandBufferInheritanceInfo* inheritanceInfo, VkCommandBufferUsageFlagBits usageFlags){
    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usageFlags;
    beginInfo.pInheritanceInfo = inheritanceInfo;

    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS){
        throw std::runtime_error("Failed to begin recording command buffer");
    }

    if(flags & COMMAND_BUFFER_GRAPHICS_FLAG == COMMAND_BUFFER_GRAPHICS_FLAG){
        pipeline->BeginRenderPassAndBindPipeline(imageIndex, commandBuffer);
    }
}

void CommandBuffer::EndCommandBuffer(){
    if(flags & COMMAND_BUFFER_GRAPHICS_FLAG == COMMAND_BUFFER_GRAPHICS_FLAG){
        pipeline->EndRenderPass(commandBuffer);
    }

    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to record command buffer");
    }

}

VkCommandBuffer CommandBuffer::GetCommandBuffer(){
    return commandBuffer;
}

CommandBuffer::CommandBuffer(const CommandBuffer& other){
    commandBuffer = other.commandBuffer;
    useCount = other.useCount;
    pipeline = other.pipeline;
    level = other.level;
    flags = other.flags;
    useCount[0]++;
}

CommandBuffer CommandBuffer::operator=(const CommandBuffer& other) {
    if(this == &other){
        return *this;
    }

    commandBuffer = other.commandBuffer;
    pipeline = other.pipeline;
    useCount = other.useCount;
    level = other.level;
    flags = other.flags;
    useCount[0]++;
    return *this;
}

CommandBuffer::~CommandBuffer(){
    if(useCount[0] <= 1){
        if(flags & COMMAND_BUFFER_GRAPHICS_FLAG == COMMAND_BUFFER_GRAPHICS_FLAG){
            vkFreeCommandBuffers(Device::GetDevice(), CommandPool::GetGraphicsCommandPool(), 1, &commandBuffer);
        }else{
            if(Device::GetQueueFamilyInfo().transferFamilyFound){
                vkFreeCommandBuffers(Device::GetDevice(), CommandPool::GetGraphicsCommandPool(), 1, &commandBuffer);
            }else{
                vkFreeCommandBuffers(Device::GetDevice(), CommandPool::GetTransferCommandPool(), 1, &commandBuffer);
            }
        }
        delete[] useCount;
    }
    else{
        useCount[0]--;
    }
}
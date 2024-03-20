#include "CommandBuffer.hpp"

CommandBuffer::CommandBuffer(): commandBuffer(VK_NULL_HANDLE) {}

CommandBuffer::CommandBuffer(VkCommandBufferLevel level, uint32_t flags, GraphicsPipeline* pipeline){
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
    this->pipeline.reset(pipeline);
    this->level = level;
}

void CommandBuffer::BeginCommandBuffer(uint32_t imageIndex){
    if(level == VK_COMMAND_BUFFER_LEVEL_SECONDARY){
        throw std::runtime_error("Tried to record a secondary command buffer, aborting!");
    }

    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS){
        throw std::runtime_error("Failed to begin recording command buffer");
    }

    if(flags & COMMAND_BUFFER_GRAPHICS_FLAG == COMMAND_BUFFER_GRAPHICS_FLAG){
        pipeline.get()->BeginRenderPassAndBindPipeline(imageIndex, commandBuffer);
    }
}

void CommandBuffer::EndCommandBuffer(){
    if(flags & COMMAND_BUFFER_GRAPHICS_FLAG == COMMAND_BUFFER_GRAPHICS_FLAG){
        pipeline.get()->EndRenderPass(commandBuffer);
    }

    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to record command buffer");
    }

}

VkCommandBuffer CommandBuffer::GetCommandBuffer(){
    return commandBuffer;
}

CommandBuffer::CommandBuffer(const CommandBuffer& other) : pipeline(other.pipeline){
    commandBuffer = other.commandBuffer;
    useCount = other.useCount;
    useCount[0]++;
}

CommandBuffer CommandBuffer::operator=(const CommandBuffer& other) {
    if(this == &other){
        return *this;
    }

    commandBuffer = other.commandBuffer;
    pipeline = other.pipeline;
    useCount = other.useCount;
    useCount[0]++;
    return *this;
}

CommandBuffer::~CommandBuffer(){
    if(useCount[0] == 1){
        vkFreeCommandBuffers(Device::GetDevice(), CommandPool::GetGraphicsCommandPool(), 1, &commandBuffer);
        delete[] useCount;
    }
    else{
        useCount[0]--;
    }
}
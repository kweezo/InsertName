#include "CommandBuffer.hpp"

CommandBuffer::CommandBuffer(VkCommandBufferLevel level, uint32_t flags, GraphicsPipeline& pipeline) : pipeline(pipeline){
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = flags == COMMAND_BUFFER_GRAPHICS_FLAG ? CommandPool::GetGraphicsCommandPool() : CommandPool::GetTransferCommandPool();
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    if(vkAllocateCommandBuffers(Device::GetDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate command buffers");
    }
}

void CommandBuffer::BeginCommandBuffer(uint32_t imageIndex){
    //TODO, HANDLE SECONDARY CMD BUFFERS

    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS){
        throw std::runtime_error("Failed to begin recording command buffer");
    }

    pipeline.BeginRenderPassAndBindPipeline(imageIndex, commandBuffer);
}

void CommandBuffer::EndCommandBuffer(){
    pipeline.EndRenderPass(commandBuffer);

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
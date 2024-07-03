#include "CommandBuffer.hpp"
namespace renderer
{

CommandBuffer::CommandBuffer() : commandBuffer(VK_NULL_HANDLE){
    useCount = std::make_shared<uint32_t>(1);
    flags = 0;
}

CommandBuffer::CommandBuffer(CommandBufferCreateInfo createInfo): flags(createInfo.flags), level(createInfo.level){

    poolID = createInfo.type * std::thread::hardware_concurrency() + createInfo.threadIndex; // TODO does this work?=????

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = createInfo.level;
    allocInfo.commandBufferCount = 1;

    if(flags & COMMAND_BUFFER_GRAPHICS_FLAG == COMMAND_BUFFER_GRAPHICS_FLAG){

        allocInfo.commandPool = CommandPool::GetGraphicsCommandPool(poolID);
        if(level == VK_COMMAND_BUFFER_LEVEL_PRIMARY){
            flags |= VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;
        }else{
            flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        }
        
    }else{
        allocInfo.commandPool = CommandPool::GetTransferCommandPool(poolID);
    }

    if (vkAllocateCommandBuffers(Device::GetDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate command buffers");
    }

    useCount = std::make_shared<uint32_t>(1);
}

void CommandBuffer::ResetCommandBuffer(){
    vkResetCommandBuffer(commandBuffer, 0);
}

void CommandBuffer::BeginCommandBuffer(VkCommandBufferInheritanceInfo *inheritanceInfo, bool reset){
    //    if(level == VK_COMMAND_BUFFER_LEVEL_SECONDARY){
    //        throw std::runtime_error("Tried to record a secondary command buffer, aborting!");
    //    }
    // I am going to leave this here commented outas a testament to my stupidity and retardation, I already removed it once after I figured out that
    // it was the cause of the problem and it came back to bite me in the ass again, I have no idea how or when it reappeared but it did
    // it is a minor inconvenience but one that will probably be mentioned in my suicide note lol
    // so done with this typa bullshit

    if (commandBuffer == VK_NULL_HANDLE){
        throw std::runtime_error("Tried to begin the recording of an uninitialized command buffer, aborting (commandBuffer = VK_NULL_HANDLE)");
    }

    if(reset){
        vkResetCommandBuffer(commandBuffer, 0);
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = inheritanceInfo;
    beginInfo.flags = ((flags & COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG) == COMMAND_BUFFER_TRANSFER_FLAG)
                          ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
                          : 0;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS){
        throw std::runtime_error("Failed to begin recording command buffer");
    }
}

void CommandBuffer::EndCommandBuffer(){
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to record command buffer");
    }
}

void CommandBuffer::ResetPools(CommandBufferType type){
    for(uint32_t i = 0; i < std::thread::hardware_concurrency(); i++){
        CommandPool::ResetPool(type * std::thread::hardware_concurrency() + i);
    }
}

VkCommandBuffer CommandBuffer::GetCommandBuffer(){
    return commandBuffer;
}

CommandBuffer::CommandBuffer(const CommandBuffer &other){
    commandBuffer = other.commandBuffer;
    useCount = other.useCount;
    flags = other.flags;
    poolID = other.poolID;
    (*useCount.get())++;
}

CommandBuffer CommandBuffer::operator=(const CommandBuffer &other){
    if (this == &other){
        return *this;
    }

    commandBuffer = other.commandBuffer;
    useCount = other.useCount;
    flags = other.flags;
    poolID = other.poolID;
    (*useCount.get())++;

    return *this;
}

CommandBuffer::~CommandBuffer(){
    if (useCount.get() == nullptr){
        return;
    }

    if (*useCount == 1){
        if ((flags & COMMAND_BUFFER_GRAPHICS_FLAG) == COMMAND_BUFFER_GRAPHICS_FLAG){
            CommandPool::FreeCommandBuffer(commandBuffer, poolID, COMMAND_POOL_TYPE_GRAPHICS);
        }
        else{
            CommandPool::FreeCommandBuffer(commandBuffer, poolID, COMMAND_POOL_TYPE_TRANSFER);
        }
        useCount.reset();
    }
    else{
        (*useCount.get())--;
    }
}

}
#include "CommandBuffer.hpp"
namespace renderer
{

std::deque<std::mutex> _CommandBuffer::poolMutexes = {};

void _CommandBuffer::Init(){
    poolMutexes.resize(_CommandBufferType::size * std::thread::hardware_concurrency());
}

_CommandBuffer::_CommandBuffer() : commandBuffer(VK_NULL_HANDLE){
    flags = 0;
}

_CommandBuffer::_CommandBuffer(_CommandBufferCreateInfo createInfo): flags(createInfo.flags), level(createInfo.level){

    poolID = createInfo.type * std::thread::hardware_concurrency() + createInfo.threadIndex; 

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = createInfo.level;
    allocInfo.commandBufferCount = 1;

    if(flags & COMMAND_BUFFER_GRAPHICS_FLAG == COMMAND_BUFFER_GRAPHICS_FLAG){

        allocInfo.commandPool = _CommandPool::GetGraphicsCommandPool(poolID);
        if(level == VK_COMMAND_BUFFER_LEVEL_PRIMARY){
            flags |= VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;
        }else{
            flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        }
        
    }else{
        allocInfo.commandPool = _CommandPool::GetTransferCommandPool(poolID);
    }

    if (vkAllocateCommandBuffers(_Device::GetDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate command buffers");
    }

    useCount = std::make_shared<uint32_t>(1);

}

void _CommandBuffer::ResetCommandBuffer(){
    vkResetCommandBuffer(commandBuffer, 0);
}


void _CommandBuffer::BeginCommandBuffer(VkCommandBufferInheritanceInfo *inheritanceInfo, bool reset){
    //    if(level == VK_COMMAND_BUFFER_LEVEL_SECONDARY){
    //        throw std::runtime_error("Tried to record a secondary command buffer, aborting!");
    //    }
    // I am going to leave this here commented outas a testament to my stupidity and retardation, I already removed it once after I figured out that
    // it was the cause of the problem and it came back to bite me in the ass again, I have no idea how or when it reappeared but it did
    // it is a minor inconvenience but one that will probably be mentioned in my suicide note lol
    // so done with this typa bullshit


     //actfuhasaally you can't use make_unique cause mutexes can't be moved 🤓
    //I'm being FORCED into this by a static assert, fuck RAII all my homies *despise* RAII(not really it just mildly inconveniences me)

    lock.reset(new std::lock_guard(poolMutexes[poolID]));

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
    beginInfo.flags = ((flags & COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG) == COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG)
                          ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
                          : 0;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS){
        throw std::runtime_error("Failed to begin recording command buffer");
    }

}

void _CommandBuffer::EndCommandBuffer(){
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to record command buffer");
    }

    lock.reset();
}

void _CommandBuffer::ResetPools(_CommandBufferType type, uint32_t threadIndex){
    _CommandPool::ResetPool(type * std::thread::hardware_concurrency() + threadIndex);

}

VkCommandBuffer _CommandBuffer::GetCommandBuffer(){
    return commandBuffer;
}

_CommandBuffer::_CommandBuffer(const _CommandBuffer &other){
    if(other.useCount.get() == nullptr){
        return;
    }


    commandBuffer = other.commandBuffer;
    useCount = other.useCount;
    flags = other.flags;
    poolID = other.poolID;
    level = other.level;

    (*useCount.get())++;
}

_CommandBuffer _CommandBuffer::operator=(const _CommandBuffer &other){
    if (this == &other){
        return *this;
    }

    if(other.useCount.get() == nullptr){
        return *this;
    }

    Destruct();

    commandBuffer = other.commandBuffer;
    useCount = other.useCount;
    flags = other.flags;
    poolID = other.poolID;
    level = other.level;

    (*useCount.get())++;

    return *this;
}

void _CommandBuffer::Destruct(){
    if (useCount.get() == nullptr){
        return;
    }

    if (*useCount == 1){
        if ((flags & COMMAND_BUFFER_GRAPHICS_FLAG) == COMMAND_BUFFER_GRAPHICS_FLAG){
            vkFreeCommandBuffers(_Device::GetDevice(), _CommandPool::GetGraphicsCommandPool(poolID), 1, &commandBuffer);
        }
        else{
            vkFreeCommandBuffers(_Device::GetDevice(), _CommandPool::GetTransferCommandPool(poolID), 1, &commandBuffer);
        }
        useCount.reset();
    }
    else{
        (*useCount.get())--;
    }
}

_CommandBuffer::~_CommandBuffer(){
   Destruct();
}

}
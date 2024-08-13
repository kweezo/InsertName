#include "CommandBuffer.hpp"

#include <thread>

namespace renderer {
    std::deque<std::mutex> i_CommandBuffer::poolMutexes = {};

    void i_CommandBuffer::Init() {
        poolMutexes.resize(i_CommandBufferType::size * std::thread::hardware_concurrency());
    }

    i_CommandBuffer::i_CommandBuffer() : level(),
                                         flags(),
                                         poolID(),
                                         threadIndex(),
                                         lock(),
                                         useCount(),
                                         commandBuffer() {
    }

    i_CommandBuffer::i_CommandBuffer(i_CommandBufferCreateInfo createInfo): level(createInfo.level),
                                                                            flags(createInfo.flags),
                                                                            poolID(),
                                                                            threadIndex(createInfo.threadIndex),
                                                                            lock(),
                                                                            useCount(),
                                                                            commandBuffer() {
        poolID = createInfo.type * std::thread::hardware_concurrency() + createInfo.threadIndex;

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = createInfo.level;
        allocInfo.commandBufferCount = 1;


        if (flags & COMMAND_BUFFER_GRAPHICS_FLAG == COMMAND_BUFFER_GRAPHICS_FLAG) {
            allocInfo.commandPool = i_CommandPool::GetGraphicsCommandPool(poolID);
            if (level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
                flags |= VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;
            } else {
                flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            }
        } else {
            allocInfo.commandPool = i_CommandPool::GetTransferCommandPool(poolID);
        }

        if (vkAllocateCommandBuffers(i_Device::GetDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers");
        }

        useCount = std::make_shared<uint32_t>(1);
    }

    void i_CommandBuffer::ResetCommandBuffer() const {
        vkResetCommandBuffer(commandBuffer, 0);
    }


    void i_CommandBuffer::BeginCommandBuffer(VkCommandBufferInheritanceInfo *inheritanceInfo, bool reset) {
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

        if (commandBuffer == VK_NULL_HANDLE) {
            throw std::runtime_error(
                "Tried to begin the recording of an uninitialized command buffer, aborting (commandBuffer = VK_NULL_HANDLE)");
        }

        if (reset) {
            vkResetCommandBuffer(commandBuffer, 0);
        }

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = inheritanceInfo;
        beginInfo.flags = ((flags & COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG) == COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG)
                              ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
                              : 0;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer");
        }
    }

    void i_CommandBuffer::EndCommandBuffer() {
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer");
        }

        lock.reset();
    }

    void i_CommandBuffer::ResetPools(i_CommandBufferType type, uint32_t threadIndex) {
        i_CommandPool::ResetPool(type * std::thread::hardware_concurrency() + threadIndex);
    }

    VkCommandBuffer i_CommandBuffer::GetCommandBuffer() const {
        return commandBuffer;
    }

    i_CommandBuffer::i_CommandBuffer(const i_CommandBuffer &other) {
        if (other.useCount == nullptr) {
            return;
        }


        commandBuffer = other.commandBuffer;
        useCount = other.useCount;
        flags = other.flags;
        poolID = other.poolID;
        level = other.level;
        threadIndex = other.threadIndex;

        (*useCount)++;
    }

    i_CommandBuffer &i_CommandBuffer::operator=(const i_CommandBuffer &other) {
        if (this == &other) {
            return *this;
        }

        if (other.useCount == nullptr) {
            return *this;
        }

        Destruct();

        commandBuffer = other.commandBuffer;
        useCount = other.useCount;
        flags = other.flags;
        poolID = other.poolID;
        level = other.level;
        threadIndex = other.threadIndex;

        (*useCount)++;

        return *this;
    }

    void i_CommandBuffer::Destruct() {
        if (useCount == nullptr) {
            return;
        }

        if (*useCount == 1) {
            if ((flags & COMMAND_BUFFER_GRAPHICS_FLAG) == COMMAND_BUFFER_GRAPHICS_FLAG) {
                vkFreeCommandBuffers(i_Device::GetDevice(), i_CommandPool::GetGraphicsCommandPool(poolID), 1,
                                     &commandBuffer);
            } else {
                vkFreeCommandBuffers(i_Device::GetDevice(), i_CommandPool::GetTransferCommandPool(poolID), 1,
                                     &commandBuffer);
            }
            useCount.reset();
        } else {
            (*useCount)--;
        }
    }

    i_CommandBuffer::~i_CommandBuffer() {
        Destruct();
    }

    uint32_t i_CommandBuffer::GetThreadIndex() const {
        return threadIndex;
    }
}

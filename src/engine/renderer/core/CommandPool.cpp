#include "CommandPool.hpp"


namespace renderer{

std::unordered_map<uint32_t, __CommandPoolSet> __CommandPool::commandPools = {};

void __CommandPool::CreateCommandPools(uint32_t poolID, uint32_t commandPoolType){
    __CommandPoolSet& set = commandPools[poolID];

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = (commandPoolType == COMMAND_POOL_TYPE_GRAPHICS) ?
     __Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex :
     __Device::GetQueueFamilyInfo().transferQueueCreateInfo.queueFamilyIndex;
    
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if(vkCreateCommandPool(__Device::GetDevice(), &poolInfo, nullptr, &set.graphicsCommandPool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create the graphics command pool!");
    }


    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if(__Device::GetQueueFamilyInfo().transferFamilyFound){
        poolInfo.queueFamilyIndex = __Device::GetQueueFamilyInfo().transferQueueCreateInfo.queueFamilyIndex;
    }

    if(vkCreateCommandPool(__Device::GetDevice(), &poolInfo, nullptr, &set.transferCommandPool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create the transfer command pool!");
    }
}

VkCommandPool __CommandPool::GetGraphicsCommandPool(uint32_t poolID){
    __CommandPoolSet& set = commandPools[poolID];
    set.commandBufferCount++;
    return set.graphicsCommandPool;
}

VkCommandPool __CommandPool::GetTransferCommandPool(uint32_t poolID){
    __CommandPoolSet& set = commandPools[poolID];
    set.commandBufferCount++;
    return set.transferCommandPool;
}

VkCommandPool __CommandPool::ResetPool(uint32_t poolID){ 
    __CommandPoolSet& set = commandPools[poolID];
    vkResetCommandPool(__Device::GetDevice(), set.graphicsCommandPool, 0);

    if(set.graphicsCommandPool != set.transferCommandPool){
        vkResetCommandPool(__Device::GetDevice(), set.transferCommandPool, 0);
    }

    return set.graphicsCommandPool;
}

void __CommandPool::FreeCommandBuffer(VkCommandBuffer commandBuffer, uint32_t poolID, uint32_t commandPoolType){
    __CommandPoolSet& set = commandPools[poolID];
    VkCommandPool commandPool = (commandPoolType == COMMAND_POOL_TYPE_GRAPHICS) ? set.graphicsCommandPool : set.transferCommandPool;

    vkFreeCommandBuffers(__Device::GetDevice(), commandPool, 1, &commandBuffer);

    set.commandBufferCount--;
    if(!set.commandBufferCount){
        vkDestroyCommandPool(__Device::GetDevice(), set.graphicsCommandPool, nullptr);
        vkDestroyCommandPool(__Device::GetDevice(), set.transferCommandPool, nullptr);
        commandPools.erase(poolID);
    }

}

void __CommandPool::Cleanup(){
    commandPools.clear();
}

}
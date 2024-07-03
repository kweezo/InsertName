#include "CommandPool.hpp"


namespace renderer{

std::unordered_map<uint32_t, CommandPoolSet> CommandPool::commandPools = {};

void CommandPool::CreateCommandPools(uint32_t poolID){
    CommandPoolSet& set = commandPools[poolID];

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if(vkCreateCommandPool(Device::GetDevice(), &poolInfo, nullptr, &set.graphicsCommandPool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create the graphics command pool!");
    }


    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if(Device::GetQueueFamilyInfo().transferFamilyFound){
        poolInfo.queueFamilyIndex = Device::GetQueueFamilyInfo().transferQueueCreateInfo.queueFamilyIndex;
    }

    if(vkCreateCommandPool(Device::GetDevice(), &poolInfo, nullptr, &set.transferCommandPool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create the transfer command pool!");
    }
}

VkCommandPool CommandPool::GetGraphicsCommandPool(uint32_t poolID){
    if(commandPools.find(poolID) == commandPools.end()){
        CreateCommandPools(poolID);
    }

    CommandPoolSet& set = commandPools[poolID];
    set.commandBufferCount++;
    return set.graphicsCommandPool;
}

VkCommandPool CommandPool::GetTransferCommandPool(uint32_t poolID){
    if(commandPools.find(poolID) == commandPools.end()){
        CreateCommandPools(poolID);
    }

    CommandPoolSet& set = commandPools[poolID];
    set.commandBufferCount++;
    return set.transferCommandPool;
}

void CommandPool::FreeCommandBuffer(VkCommandBuffer commandBuffer, uint32_t poolID, uint32_t commandPoolType){
    CommandPoolSet& set = commandPools[poolID];
    VkCommandPool commandPool = (commandPoolType == COMMAND_POOL_TYPE_GRAPHICS) ? set.graphicsCommandPool : set.transferCommandPool;

    vkFreeCommandBuffers(Device::GetDevice(), commandPool, 1, &commandBuffer);

    set.commandBufferCount--;
    if(!set.commandBufferCount){
        vkDestroyCommandPool(Device::GetDevice(), set.graphicsCommandPool, nullptr);
        vkDestroyCommandPool(Device::GetDevice(), set.transferCommandPool, nullptr);
        commandPools.erase(poolID);
    }

}


}
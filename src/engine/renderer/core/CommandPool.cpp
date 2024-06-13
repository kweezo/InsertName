#include "CommandPool.hpp"


namespace renderer{

std::unordered_map<std::thread::id, CommandPoolSet> CommandPool::commandPools = {};

void CommandPool::CreateCommandPools(std::thread::id threadID){
    CommandPoolSet& set = commandPools[threadID];

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;;

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

VkCommandPool CommandPool::GetGraphicsCommandPool(std::thread::id threadID){
    if(commandPools.find(threadID) == commandPools.end()){
        CreateCommandPools(threadID);
    }
    CommandPoolSet& set = commandPools[threadID];
    set.commandBufferCount++;
    return set.graphicsCommandPool;
}

VkCommandPool CommandPool::GetTransferCommandPool(std::thread::id threadID){
    if(commandPools.find(threadID) == commandPools.end()){
        CreateCommandPools(threadID);
    }
    CommandPoolSet& set = commandPools[threadID];
    set.commandBufferCount++;
    return set.transferCommandPool;
}

void CommandPool::NotifyCommandBufferDestruction(std::thread::id threadID){
    CommandPoolSet& set = commandPools[threadID];
    set.commandBufferCount--;
    if(!set.commandBufferCount){
        vkDestroyCommandPool(Device::GetDevice(), set.graphicsCommandPool, nullptr);
        vkDestroyCommandPool(Device::GetDevice(), set.transferCommandPool, nullptr);
        commandPools.erase(threadID);
    }
}


}
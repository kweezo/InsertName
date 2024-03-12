#include "CommandPool.hpp"

VkCommandPool CommandPool::graphicsCommandPool = VK_NULL_HANDLE;
VkCommandPool CommandPool::transferCommandPool = VK_NULL_HANDLE;

void CommandPool::CreateCommandPools(){
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // MAYBE DONT RESET IT??

    if(vkCreateCommandPool(Device::GetDevice(), &poolInfo, nullptr, &graphicsCommandPool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create the graphics command pool!");
    }

    if(Device::GetQueueFamilyInfo().transferFamilyFound){
        poolInfo.queueFamilyIndex = Device::GetQueueFamilyInfo().transferQueueCreateInfo.queueFamilyIndex;

        if(vkCreateCommandPool(Device::GetDevice(), &poolInfo, nullptr, &transferCommandPool) != VK_SUCCESS){
            throw std::runtime_error("Failed to create the transfer command pool!");
        }
    }else{
        transferCommandPool = graphicsCommandPool;
    }
}

VkCommandPool CommandPool::GetGraphicsCommandPool(){
    return graphicsCommandPool;
}

VkCommandPool CommandPool::GetTransferCommandPool(){
    return transferCommandPool;
}

void CommandPool::DestroyCommandPools(){
    vkDestroyCommandPool(Device::GetDevice(), graphicsCommandPool, nullptr);
    if(Device::GetQueueFamilyInfo().transferFamilyFound){
        vkDestroyCommandPool(Device::GetDevice(), transferCommandPool, nullptr);
    }
}
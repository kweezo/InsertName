#include "CommandPool.hpp"


namespace renderer{

std::vector<i_CommandPoolSet> i_CommandPool::commandPools{};


void i_CommandPool::Init(){
    commandPools.resize(std::thread::hardware_concurrency() * i_CommandBufferType::size);

    for(uint32_t i = 0; i < commandPools.size(); i++){
        CreateCommandPool(i);
    }
}

void i_CommandPool::CreateCommandPool(uint32_t poolID){
    i_CommandPoolSet& set = commandPools[poolID];

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = i_Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex;

    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if(vkCreateCommandPool(i_Device::GetDevice(), &poolInfo, nullptr, &set.graphicsCommandPool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create the graphics command pool!");
    }

    if(!i_Device::GetQueueFamilyInfo().transferFamilyFound){
        set.transferCommandPool = set.graphicsCommandPool;
        return;
    }

    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if(i_Device::GetQueueFamilyInfo().transferFamilyFound){
        poolInfo.queueFamilyIndex = i_Device::GetQueueFamilyInfo().transferQueueCreateInfo.queueFamilyIndex;
    }

    if(vkCreateCommandPool(i_Device::GetDevice(), &poolInfo, nullptr, &set.transferCommandPool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create the transfer command pool!");
    }
}

VkCommandPool i_CommandPool::GetGraphicsCommandPool(uint32_t poolID){
    i_CommandPoolSet& set = commandPools[poolID];

    return set.graphicsCommandPool;
}

VkCommandPool i_CommandPool::GetTransferCommandPool(uint32_t poolID){
    i_CommandPoolSet& set = commandPools[poolID];

    return set.transferCommandPool;
}

VkCommandPool i_CommandPool::ResetPool(uint32_t poolID){ 
    i_CommandPoolSet& set = commandPools[poolID];
    vkResetCommandPool(i_Device::GetDevice(), set.graphicsCommandPool, 0);

    if(set.graphicsCommandPool != set.transferCommandPool){
        vkResetCommandPool(i_Device::GetDevice(), set.transferCommandPool, 0);
    }

    return set.graphicsCommandPool;
}

void i_CommandPool::Cleanup(){
    for(i_CommandPoolSet& commandPoolSet : commandPools){
        if(commandPoolSet.graphicsCommandPool != commandPoolSet.transferCommandPool){
            vkDestroyCommandPool(i_Device::GetDevice(), commandPoolSet.transferCommandPool, nullptr);
        }
        vkDestroyCommandPool(i_Device::GetDevice(), commandPoolSet.graphicsCommandPool, nullptr);
    }
}

}
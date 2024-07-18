#include "CommandPool.hpp"


namespace renderer{

std::vector<__CommandPoolSet> __CommandPool::commandPools{};


void __CommandPool::Init(){
    commandPools.resize(std::thread::hardware_concurrency() * __CommandBufferType::size);

    for(uint32_t i = 0; i < commandPools.size(); i++){
        CreateCommandPool(i);
    }
}

void __CommandPool::CreateCommandPool(uint32_t poolID){
    __CommandPoolSet& set = commandPools[poolID];

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = __Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex;

    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if(vkCreateCommandPool(__Device::GetDevice(), &poolInfo, nullptr, &set.graphicsCommandPool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create the graphics command pool!");
    }

    if(!__Device::GetQueueFamilyInfo().transferFamilyFound){
        set.transferCommandPool = set.graphicsCommandPool;
        return;
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

    return set.graphicsCommandPool;
}

VkCommandPool __CommandPool::GetTransferCommandPool(uint32_t poolID){
    __CommandPoolSet& set = commandPools[poolID];

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

void __CommandPool::Cleanup(){
    for(__CommandPoolSet& commandPoolSet : commandPools){
        if(commandPoolSet.graphicsCommandPool != commandPoolSet.transferCommandPool){
            vkDestroyCommandPool(__Device::GetDevice(), commandPoolSet.transferCommandPool, nullptr);
        }
        vkDestroyCommandPool(__Device::GetDevice(), commandPoolSet.graphicsCommandPool, nullptr);
    }
}

}
#include "CommandPool.hpp"


namespace renderer{

std::vector<_CommandPoolSet> _CommandPool::commandPools{};


void _CommandPool::Init(){
    commandPools.resize(std::thread::hardware_concurrency() * _CommandBufferType::size);

    for(uint32_t i = 0; i < commandPools.size(); i++){
        CreateCommandPool(i);
    }
}

void _CommandPool::CreateCommandPool(uint32_t poolID){
    _CommandPoolSet& set = commandPools[poolID];

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = _Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex;

    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if(vkCreateCommandPool(_Device::GetDevice(), &poolInfo, nullptr, &set.graphicsCommandPool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create the graphics command pool!");
    }

    if(!_Device::GetQueueFamilyInfo().transferFamilyFound){
        set.transferCommandPool = set.graphicsCommandPool;
        return;
    }

    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if(_Device::GetQueueFamilyInfo().transferFamilyFound){
        poolInfo.queueFamilyIndex = _Device::GetQueueFamilyInfo().transferQueueCreateInfo.queueFamilyIndex;
    }

    if(vkCreateCommandPool(_Device::GetDevice(), &poolInfo, nullptr, &set.transferCommandPool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create the transfer command pool!");
    }
}

VkCommandPool _CommandPool::GetGraphicsCommandPool(uint32_t poolID){
    _CommandPoolSet& set = commandPools[poolID];

    return set.graphicsCommandPool;
}

VkCommandPool _CommandPool::GetTransferCommandPool(uint32_t poolID){
    _CommandPoolSet& set = commandPools[poolID];

    return set.transferCommandPool;
}

VkCommandPool _CommandPool::ResetPool(uint32_t poolID){ 
    _CommandPoolSet& set = commandPools[poolID];
    vkResetCommandPool(_Device::GetDevice(), set.graphicsCommandPool, 0);

    if(set.graphicsCommandPool != set.transferCommandPool){
        vkResetCommandPool(_Device::GetDevice(), set.transferCommandPool, 0);
    }

    return set.graphicsCommandPool;
}

void _CommandPool::Cleanup(){
    for(_CommandPoolSet& commandPoolSet : commandPools){
        if(commandPoolSet.graphicsCommandPool != commandPoolSet.transferCommandPool){
            vkDestroyCommandPool(_Device::GetDevice(), commandPoolSet.transferCommandPool, nullptr);
        }
        vkDestroyCommandPool(_Device::GetDevice(), commandPoolSet.graphicsCommandPool, nullptr);
    }
}

}
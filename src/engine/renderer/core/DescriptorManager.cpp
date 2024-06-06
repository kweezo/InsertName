#include "DescriptorManager.hpp"

namespace renderer{

std::vector<VkDescriptorSetLayout> DescriptorManager::layouts = {};
std::vector<DescriptorBatch> DescriptorManager::batches = {};

std::vector<uint32_t> DescriptorManager::CreateLayouts(std::vector<VkDescriptorSetLayoutCreateInfo>& layoutInfos){
    size_t oldLayoutsSize = layouts.size();
    layouts.resize(layoutInfos.size() + layouts.size());
    std::vector<uint32_t> layoutIndexes;
    for(uint32_t i = oldLayoutsSize; i < layouts.size(); i++){
        if(vkCreateDescriptorSetLayout(Device::GetDevice(), &layoutInfos[i-oldLayoutsSize], nullptr, &layouts[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create descriptor set layout");
        }
        layoutIndexes.push_back(i);
    }

    return layoutIndexes;
}

std::vector<DescriptorHandle> DescriptorManager::CreateDescriptors(std::vector<DescriptorBatchInfo> batchInfos,
    std::vector<uint32_t> layoutIndex){
    uint32_t descriptorCount = 0;

    std::vector<VkDescriptorPoolSize> poolSizes;
    for(const DescriptorBatchInfo& batchInfo : batchInfos){
        poolSizes.push_back({batchInfo.descriptorType, batchInfo.setCount});
        descriptorCount += batchInfo.setCount;
    }

    batches.resize(batches.size()+1);

    DescriptorBatch& batch = batches.back();


    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = descriptorCount;

    if(vkCreateDescriptorPool(Device::GetDevice(), &poolInfo, nullptr, &batch.pool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create descriptor pool");
    }

    uint32_t i = 0;

    std::vector<VkDescriptorSetLayout> indexLayouts;
    for(uint32_t index : layoutIndex){
        indexLayouts.push_back(layouts[index]);
    }

    batch.sets.resize(descriptorCount);
    std::vector<DescriptorHandle> handles(descriptorCount);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = batch.pool;
    allocInfo.descriptorSetCount = descriptorCount;
    allocInfo.pSetLayouts = indexLayouts.data();

    handles[i].batchIndex = batches.size()-1;
    handles[i].index = i;

    if(vkAllocateDescriptorSets(Device::GetDevice(), &allocInfo, batch.sets.data()+(i * sizeof(VkDescriptorSet))) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate descriptor sets");
    }

    return handles;
}

VkDescriptorSet DescriptorManager::GetDescriptorSet(DescriptorHandle handles){
    return batches[handles.batchIndex].sets[handles.index];
}

std::vector<VkDescriptorSetLayout>* DescriptorManager::GetLayouts(){
    return &layouts;
}

void DescriptorManager::Cleanup(){
    for(VkDescriptorSetLayout layout : layouts){
        vkDestroyDescriptorSetLayout(Device::GetDevice(), layout, nullptr);
    }

    for(DescriptorBatch& batch : batches){
        vkDestroyDescriptorPool(Device::GetDevice(), batch.pool, nullptr);
    }
}

}

#include "DescriptorManager.hpp"

namespace renderer{

std::vector<VkDescriptorSetLayout> DescriptorManager::layouts = {};
std::vector<DescriptorBatch> DescriptorManager::batches = {};

std::vector<uint32_t> DescriptorManager::CreateLayouts(std::vector<VkDescriptorSetLayoutCreateInfo> layoutInfos){
    int i = layouts.size();
    std::vector<uint32_t> indices(layoutInfos.size());
    layouts.resize(layouts.size()+layoutInfos.size());
    for(const VkDescriptorSetLayoutCreateInfo& layoutInfo : layoutInfos){
        indices[layouts.size()-i-1] = i;
        if(vkCreateDescriptorSetLayout(Device::GetDevice(), &layoutInfo, nullptr, &layouts[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create descriptor set layout");
        }
        i++;
    }

    return indices;
}

std::vector<DescriptorHandle> DescriptorManager::CreateDescriptors(std::vector<DescriptorBatchInfo> batchInfos, uint32_t setCount,
uint32_t layoutIndex){
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

    batch.sets.resize(setCount);
    std::vector<DescriptorHandle> handles(setCount);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = batch.pool;
    allocInfo.descriptorSetCount = setCount;
    allocInfo.pSetLayouts = &layouts[layoutIndex];

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

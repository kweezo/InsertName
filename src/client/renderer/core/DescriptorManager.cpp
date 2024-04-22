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

std::vector<DescriptorHandle> DescriptorManager::CreateDescriptors(std::vector<DescriptorBatchInfo> batchInfos){
    uint32_t setCount = 0;
    for(const DescriptorBatchInfo& batchInfo : batchInfos){
        setCount += batchInfo.setCount;
    }

    batches.resize(batches.size()+1);

    DescriptorBatch& batch = batches.back();

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = setCount;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = setCount;

    if(vkCreateDescriptorPool(Device::GetDevice(), &poolInfo, nullptr, &batch.pool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create descriptor pool");
    }

    uint32_t i = 0;

    batch.sets.resize(setCount);
    std::vector<DescriptorHandle> handles(setCount);
    for(const DescriptorBatchInfo& batchInfo : batchInfos){
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = batch.pool;
        allocInfo.descriptorSetCount = batchInfo.setCount;
        allocInfo.pSetLayouts = &layouts[batchInfo.layoutIndex];

        handles[i].batchIndex = batches.size()-1;
        handles[i].index = i;

        if(vkAllocateDescriptorSets(Device::GetDevice(), &allocInfo, batch.sets.data()+(i * sizeof(VkDescriptorSet))) != VK_SUCCESS){
            throw std::runtime_error("Failed to allocate descriptor sets");
        }

        i += batchInfo.setCount;
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

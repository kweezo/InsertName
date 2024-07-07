#include "DescriptorManager.hpp"

namespace renderer{

const uint32_t DESCRIPTOR_TYPE_COUNT = 10;

std::list<VkDescriptorPool> __DescriptorManager::descriptorPools = {};
std::vector<VkDescriptorSetLayout> __DescriptorManager::descriptorLayouts = {};
boost::container::flat_map<VkDescriptorPool, std::vector<VkDescriptorSet>> __DescriptorManager::descriptorSets = {};

std::vector<__DescriptorSetLocation> __DescriptorManager::AllocateDescriptorSetBatch(__DescriptorSetBatchAllocateInfo allocInfo){

    std::vector<VkDescriptorSetLayout> currentDescriptorLayouts(allocInfo.descriptorLayoutBindings.size());

    for(uint32_t i = 0; i < allocInfo.descriptorLayoutBindings.size(); i++){
        
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.pBindings = allocInfo.descriptorLayoutBindings[i].data();
        layoutCreateInfo.bindingCount = allocInfo.descriptorLayoutBindings[i].size();

        VkDescriptorSetLayout layout;
        if(vkCreateDescriptorSetLayout(__Device::GetDevice(), &layoutCreateInfo, nullptr, &layout) != VK_SUCCESS){
            throw std::runtime_error("Failed to create a descriptor layout");
        }

        descriptorLayouts.push_back(layout);
        currentDescriptorLayouts[i] = layout;
    }


    std::vector<VkDescriptorPoolSize> descriptorPoolSizes(DESCRIPTOR_TYPE_COUNT);
    for(uint32_t i = 0; i < DESCRIPTOR_TYPE_COUNT; i++){
        descriptorPoolSizes[i].type = (VkDescriptorType)i;
        descriptorPoolSizes[i].descriptorCount = 0;
    }

    for(std::vector<VkDescriptorSetLayoutBinding>& bindings : allocInfo.descriptorLayoutBindings){
        for(VkDescriptorSetLayoutBinding& binding : bindings){
            descriptorPoolSizes[binding.descriptorType].descriptorCount++;
        }
    }

    for(uint32_t i = 0; i < descriptorPoolSizes.size(); i++){
        if(descriptorPoolSizes[i].descriptorCount == 0){
            descriptorPoolSizes.erase(descriptorPoolSizes.begin() + i);
            i--;
        }
    }

    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.maxSets = allocInfo.descriptorLayoutBindings.size();
    poolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
    poolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

    descriptorPools.push_front({});
    if(vkCreateDescriptorPool(__Device::GetDevice(), &poolCreateInfo, nullptr, &descriptorPools.front()) != VK_SUCCESS){
        throw std::runtime_error("Failed to create a descriptor pool");
    }
    descriptorSets[descriptorPools.back()].resize(allocInfo.descriptorLayoutBindings.size());


    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = descriptorPools.front();
    descriptorSetAllocInfo.descriptorSetCount = currentDescriptorLayouts.size();
    descriptorSetAllocInfo.pSetLayouts = currentDescriptorLayouts.data();

    if(vkAllocateDescriptorSets(__Device::GetDevice(), &descriptorSetAllocInfo, descriptorSets[descriptorPools.front()].data()) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate descriptor sets");
    }

    std::vector<__DescriptorSetLocation> descriptorSetLocations(currentDescriptorLayouts.size());

    for(uint32_t i = 0; i < descriptorSetLocations.size(); i++){
        descriptorSetLocations[i].key = (uint64_t)descriptorPools.front();
        descriptorSetLocations[i].index = i;
    }

    return descriptorSetLocations;
}

std::vector<VkDescriptorSetLayout> __DescriptorManager::GetLayouts(){
    return descriptorLayouts;
}

VkDescriptorSet __DescriptorManager::RetrieveDescriptorSet(__DescriptorSetLocation location){
    return descriptorSets[(VkDescriptorPool)location.key][location.index];
}

void __DescriptorManager::Cleanup(){
    for(VkDescriptorPool descriptorPool : descriptorPools){
        vkDestroyDescriptorPool(__Device::GetDevice(), descriptorPool, nullptr);
    }

    for(VkDescriptorSetLayout descriptorLayout : descriptorLayouts){
        vkDestroyDescriptorSetLayout(__Device::GetDevice(), descriptorLayout, nullptr);
    }
}

}
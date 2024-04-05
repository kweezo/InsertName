#include "DescriptorManager.hpp"

namespace renderer{

std::vector<VkDescriptorSetLayout> DescriptorManager::layouts = {};

void DescriptorManager::Initialize(std::vector<VkDescriptorSetLayoutCreateInfo> layoutInfos){
    layouts.resize(layoutInfos.size());
    for(const VkDescriptorSetLayoutCreateInfo& layoutInfo : layoutInfos){
        VkDescriptorSetLayout layout;
        if(vkCreateDescriptorSetLayout(Device::GetDevice(), &layoutInfo, nullptr, &layout) != VK_SUCCESS){
            throw std::runtime_error("Failed to create descriptor set layout");
        }
        layouts.push_back(layout);
    }
}

}
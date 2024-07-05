#pragma once

#include <vector>
#include <list>
#include <utility>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include <boost/container/flat_map.hpp>

#include "Device.hpp"

namespace renderer{

struct __DescriptorSetLocation{
    uint64_t key;
    uint32_t index;
};

struct __DescriptorSetBatchAllocateInfo{
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorLayoutBindings;    
};

class __DescriptorManager {
public:

    static std::vector<__DescriptorSetLocation> AllocateDescriptorSetBatch(__DescriptorSetBatchAllocateInfo allocInfo);
    static VkDescriptorSet RetrieveDescriptorSet(__DescriptorSetLocation location);
    
    static std::vector<VkDescriptorSetLayout> GetLayouts();

    static void Cleanup();
private:
    static std::list<VkDescriptorPool> descriptorPools;
    static std::vector<VkDescriptorSetLayout> descriptorLayouts;
    static boost::container::flat_map<VkDescriptorPool, std::vector<VkDescriptorSet>> descriptorSets;
};

}
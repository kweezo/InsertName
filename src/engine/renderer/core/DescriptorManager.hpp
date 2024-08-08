#pragma once

#include <vector>
#include <list>
#include <utility>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include <boost/container/flat_map.hpp>

#include "Device.hpp"

namespace renderer{

struct i_DescriptorSetLocation{
    uint64_t key;
    uint32_t index;
};

struct i_DescriptorSetBatchAllocateInfo{
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorLayoutBindings;    
};

class i_DescriptorManager {
public:

    static std::vector<i_DescriptorSetLocation> AllocateDescriptorSetBatch(i_DescriptorSetBatchAllocateInfo allocInfo);
    static VkDescriptorSet RetrieveDescriptorSet(i_DescriptorSetLocation location);
    
    static std::vector<VkDescriptorSetLayout> GetLayouts();

    static void Cleanup();
private:
    static std::list<VkDescriptorPool> descriptorPools;
    static std::vector<VkDescriptorSetLayout> descriptorLayouts;
    static boost::container::flat_map<VkDescriptorPool, std::vector<VkDescriptorSet>> descriptorSets;
};

}
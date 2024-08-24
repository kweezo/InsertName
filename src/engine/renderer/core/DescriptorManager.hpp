#pragma once

#include <vector>
#include <list>
#include <utility>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include <boost/container/flat_map.hpp>

#include "Device.hpp"

namespace renderer{

struct _DescriptorSetLocation{
    uint64_t key;
    uint32_t index;
};

struct _DescriptorSetBatchAllocateInfo{
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorLayoutBindings;    
};

class _DescriptorManager {
public:

    static std::vector<_DescriptorSetLocation> AllocateDescriptorSetBatch(_DescriptorSetBatchAllocateInfo allocInfo);
    static VkDescriptorSet RetrieveDescriptorSet(_DescriptorSetLocation location);
    
    static std::vector<VkDescriptorSetLayout> GetLayouts();

    static void Cleanup();
private:
    static std::list<VkDescriptorPool> descriptorPools;
    static std::vector<VkDescriptorSetLayout> descriptorLayouts;
    static boost::container::flat_map<VkDescriptorPool, std::vector<VkDescriptorSet>> descriptorSets;
};

}
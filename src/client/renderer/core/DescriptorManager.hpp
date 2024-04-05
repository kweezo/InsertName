#pragma once

#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace renderer{

typedef struct DescriptorBatch{
    VkDescriptorSetLayout layout;
    VkDescriptorPool pool;
    std::vector<VkDescriptorSet> sets;
} DescriptorBatch;

class DescriptorManager{
public:
    static void Initialize(std::vector<VkDescriptorSetLayoutCreateInfo> layoutInfos);
    static void CreateDescriptors(std::vector<uint32_t> layoutIndexes);
    static void CreateDescriptor(uint32_t layoutIndex); //TODO implement once I figure out how to batch this mf
private:
    static std::vector<VkDescriptorSetLayout> layouts;
    static std::vector<DescriptorBatch> batches;
};

}
#pragma once

#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace renderer{

typedef struct {
    VkDescriptorSetLayout layout;
    VkDescriptorPool pool;
    std::vector<VkDescriptorSet> sets;
}DescriptorBatch;

typedef struct {
    uint32_t layoutIndex;
    uint32_t setCount;
}DescriptorBatchInfo;

typedef struct{
    uint32_t batchIndex;
    uint32_t index;
}DescriptorHandle;

class DescriptorManager{
public:
    static std::vector<uint32_t> CreateLayouts(std::vector<VkDescriptorSetLayoutCreateInfo> layoutInfos);
    static std::vector<DescriptorHandle> CreateDescriptors(std::vector<DescriptorBatchInfo> batchInfos); //TODO implement support for othre VkDescriptorTypes
    static void CreateDescriptor(uint32_t layoutIndex); //TODO implement once I figure out how to batch this mf

    static VkDescriptorSet GetDescriptorSet(DescriptorHandle handles);

    static std::vector<VkDescriptorSetLayout>* GetLayouts();

    static void Cleanup();
private:
    static std::vector<VkDescriptorSetLayout> layouts;
    static std::vector<DescriptorBatch> batches;
};

}
#pragma once

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "DescriptorManager.hpp"

#define ShaderHandle ShaderImpl*

namespace renderer{

class ShaderImpl;

typedef struct ShaderBindingInfo{
    ShaderHandle handle;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
}ShaderBindingInfo;

class Shader{
public:
    static ShaderHandle CreateShader(const char* vertexShaderPath, const char* fragmentShaderPath, std::vector<VkDescriptorSetLayoutBinding> bindings);
    static void Free(ShaderHandle shader);

    static void EnableNewShaders();
};


class ShaderImpl{
public:
    ShaderImpl(const char* vertexShaderPath, const char* fragmentShaderPath, std::vector<VkDescriptorSetLayoutBinding> bindings);
    ~ShaderImpl();
    ShaderImpl(const ShaderImpl& other);
    ShaderImpl operator=(const ShaderImpl& other);

    VkShaderModule GetVertexShaderModule() const;
    VkShaderModule GetFragmentShaderModule() const;

    void SetDescriptorSet(VkDescriptorSet descriptorSet);
    VkDescriptorSet GetDescriptorSet() const;

    void UpdateDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets);

    void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

    std::array<VkPipelineShaderStageCreateInfo, 2> GetShaderStageCreateInfo() const;

    static void EnableNewShaders();
private:
    static std::vector<ShaderBindingInfo> shaderBindings;

    std::vector<unsigned char> ReadBytecode(const char* path); 

    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;

    VkDescriptorSet descriptorSet;

    uint32_t* useCount;

};

}
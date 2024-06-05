#pragma once

#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <unordered_map>
#include <jsoncpp/json/json.h>

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
    static void Initialize();
    static void Cleanup();

    static ShaderHandle GetShader(std::string name);

private:

    static std::unordered_map<std::string, ShaderHandle> shaders;
};


class ShaderImpl{
public:
    ShaderImpl(const char* vertexShaderPath, const char* fragmentShaderPath, const char* name, 
     std::vector<VkDescriptorSetLayoutBinding> bindings);    
    ~ShaderImpl();
    ShaderImpl(const ShaderImpl& other);
    ShaderImpl operator=(const ShaderImpl& other);

    VkShaderModule GetVertexShaderModule() const;
    VkShaderModule GetFragmentShaderModule() const;
    const char* GetName();

    void SetDescriptorSet(VkDescriptorSet descriptorSet);
    void UpdateDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets);
    VkDescriptorSet GetDescriptorSet() const;

    void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

    std::array<VkPipelineShaderStageCreateInfo, 2> GetShaderStageCreateInfo() const;

    static void EnableNewShaders();
private:
    static std::vector<ShaderBindingInfo> shaderBindings;

    std::vector<unsigned char> ReadBytecode(const char* path); 

    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;

    VkDescriptorSet descriptorSet;

    const char* name;

    uint32_t* useCount;

};

}
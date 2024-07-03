#pragma once

#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <unordered_map>
#include <memory>

#include <jsoncpp/json/json.h>

#include <boost/container/flat_map.hpp>

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "DescriptorManager.hpp"

namespace renderer{

class Shader;

typedef struct ShaderBindingInfo{
    Shader* handle;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
}ShaderBindingInfo;

class ShaderManager{
public:
    static void Init();
    static void Cleanup();

    static Shader GetShader(std::string name);

private:

    static boost::container::flat_map<std::string, Shader> shaders;
};


class Shader{
public:
    Shader();
    Shader(const std::string vertexShaderPath, const std::string fragmentShaderPath, const std::string name, 
     std::vector<VkDescriptorSetLayoutBinding> bindings);    
    Shader(const Shader& other);
    Shader operator=(const Shader& other);
    ~Shader();

    VkShaderModule GetVertexShaderModule() const;
    VkShaderModule GetFragmentShaderModule() const;
    const std::string GetName();

    void SetDescriptorSet(VkDescriptorSet descriptorSet);
    void UpdateDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets);
    VkDescriptorSet GetDescriptorSet() const;

    void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

    std::array<VkPipelineShaderStageCreateInfo, 2> GetShaderStageCreateInfo() const;

    static void CreateDescriptorSets();
private:
    static std::vector<ShaderBindingInfo> shaderBindings;

    std::vector<unsigned char> ReadBytecode(const std::string path); 

    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;

    VkDescriptorSet descriptorSet;

    std::string name;

    std::shared_ptr<uint32_t> useCount;

};

}
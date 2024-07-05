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
#include "GraphicsPipeline.hpp"

#define __VertexInputDescriptions std::pair<std::vector<VkVertexInputAttributeDescription>, std::vector<VkVertexInputBindingDescription>>

namespace renderer{

class __Shader;

typedef struct __ShaderBindingInfo{
    __Shader* handle;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
}__ShaderBindingInfo;

class ShaderManager{
public:
    static void Init();
    static void Cleanup();

    static __Shader* GetShader(std::string name);

private:

    static boost::container::flat_map<std::string, __Shader> shaders;
};


class __Shader{
public:
    __Shader();
    __Shader(const std::string vertexShaderPath, const std::string fragmentShaderPath, const std::string name, 
     std::vector<VkDescriptorSetLayoutBinding> bindings);    
    __Shader(const __Shader& other);
    __Shader operator=(const __Shader& other);
    ~__Shader();

    void CreateGraphicsPipepeline(__VertexInputDescriptions vertexInputDescriptions);
    __GraphicsPipeline* GetGraphicsPipeline();

    VkShaderModule GetVertexShaderModule() const;
    VkShaderModule GetFragmentShaderModule() const;
    const std::string GetName();

    void SetDescriptorSet(VkDescriptorSet descriptorSet);
    void UpdateDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets);
    VkDescriptorSet GetDescriptorSet() const;

    void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);


    static void CreateDescriptorSets();
private:
    static std::vector<__ShaderBindingInfo> shaderBindings;


    std::vector<unsigned char> ReadBytecode(const std::string path); 

    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;

    VkDescriptorSet descriptorSet;

    std::string name;

    __GraphicsPipeline graphicsPipeline;
    std::array<VkPipelineShaderStageCreateInfo, 2> GetShaderStageCreateInfo() const;

    std::shared_ptr<uint32_t> useCount;

};

}
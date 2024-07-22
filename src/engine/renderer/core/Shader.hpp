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

class _Shader;

typedef struct _ShaderBindingInfo{
    _Shader* handle;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
}_ShaderBindingInfo;

class _ShaderManager{
public:
    static void Init();
    static void Cleanup();

    static std::shared_ptr<_Shader> GetShader(std::string name);

private:

    static boost::container::flat_map<std::string, std::shared_ptr<_Shader>> shaders;
};


class _Shader{
public:
    _Shader();
    _Shader(const std::string vertexShaderPath, const std::string fragmentShaderPath, const std::string name, 
     std::vector<VkDescriptorSetLayoutBinding> bindings, __VertexInputDescriptions vertexInputDescriptions);    

    _Shader(const _Shader& other) = delete;
    _Shader operator=(const _Shader& other) = delete;
    _Shader(_Shader&& other) = delete;
    _Shader& operator=(_Shader&& other) = delete;

    ~_Shader();

    _GraphicsPipeline* GetGraphicsPipeline();

    VkShaderModule GetVertexShaderModule() const;
    VkShaderModule GetFragmentShaderModule() const;
    const std::string GetName();

    void SetDescriptorSet(VkDescriptorSet descriptorSet);
    void UpdateDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets);
    VkDescriptorSet GetDescriptorSet() const;

    void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

    void CreateGraphicsPipepeline();

    static void CreateDescriptorSets();
private:

    static std::vector<_ShaderBindingInfo> shaderBindings;


    std::vector<unsigned char> ReadBytecode(const std::string path); 

    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;

    __VertexInputDescriptions vertexInputDescriptions;

    VkDescriptorSet descriptorSet;

    std::string name;

    _GraphicsPipeline graphicsPipeline;
    std::array<VkPipelineShaderStageCreateInfo, 2> GetShaderStageCreateInfo() const;

    std::shared_ptr<uint32_t> useCount;

};

}
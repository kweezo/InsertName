#pragma once

#include <algorithm>
#include <fstream>
#include <vector>
#include <array>
#include <memory>

#include <jsoncpp/json/json.h>

#include <boost/container/flat_map.hpp>

#include <vulkan/vulkan.h>

#include "GraphicsPipeline.hpp"

#define i_VertexInputDescriptions std::pair<std::vector<VkVertexInputAttributeDescription>, std::vector<VkVertexInputBindingDescription>>

namespace renderer {
    class i_Shader;

    struct i_ShaderBindingInfo {
        i_Shader *handle;
        std::vector<VkDescriptorSetLayoutBinding> bindings;
    };

    class i_ShaderManager {
    public:
        static void Init();

        static void Cleanup();

        static std::vector<std::weak_ptr<i_Shader> > GetShaderCategory(std::string category);

        static std::weak_ptr<i_Shader> GetShader(std::string name);

    private:
        static boost::container::flat_map<std::string, std::shared_ptr<i_Shader> > shaders;
        static boost::container::flat_map<std::string, std::vector<std::weak_ptr<i_Shader> > > shadersByCategory;
    };


    class i_Shader {
    public:
        i_Shader();

        i_Shader(std::string vertexShaderPath, std::string fragmentShaderPath, std::string name,
                 std::vector<VkDescriptorSetLayoutBinding> bindings, i_VertexInputDescriptions vertexInputDescriptions);

        i_Shader(const i_Shader &other) = delete;

        i_Shader operator=(const i_Shader &other) = delete;

        i_Shader(i_Shader &&other) = delete;

        i_Shader &operator=(i_Shader &&other) = delete;

        ~i_Shader();

        i_GraphicsPipeline *GetGraphicsPipeline();

        [[nodiscard]] VkShaderModule GetVertexShaderModule() const;

        [[nodiscard]] VkShaderModule GetFragmentShaderModule() const;

        const std::string GetName();

        void SetDescriptorSet(VkDescriptorSet descriptorSet);

        void UpdateDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets);

        [[nodiscard]] VkDescriptorSet GetDescriptorSet() const;

        void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

        void CreateGraphicsPipepeline();

        static void CreateDescriptorSets();

    private:
        static std::vector<i_ShaderBindingInfo> shaderBindings;


        std::vector<unsigned char> ReadBytecode(std::string path);

        VkShaderModule vertexShaderModule;
        VkShaderModule fragmentShaderModule;

        i_VertexInputDescriptions vertexInputDescriptions;

        VkDescriptorSet descriptorSet;

        std::string name;

        i_GraphicsPipeline graphicsPipeline;

        [[nodiscard]] std::array<VkPipelineShaderStageCreateInfo, 2> GetShaderStageCreateInfo() const;

        std::shared_ptr<uint32_t> useCount;
    };
}

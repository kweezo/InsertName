#pragma once

#include <array>
#include <memory>
#include <memory>
#include <mutex>

#include <vulkan/vulkan.h>

#include "DescriptorManager.hpp"
#include "Swapchain.hpp"

namespace renderer {
    struct i_GraphicsPipelineCreateInfo {
        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStageCreateInfo;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    };

    class i_GraphicsPipeline {
    public:
        static void Init();

        static void Cleanup();

        i_GraphicsPipeline();

        i_GraphicsPipeline(i_GraphicsPipelineCreateInfo createInfo);

        ~i_GraphicsPipeline();

        i_GraphicsPipeline(const i_GraphicsPipeline &other);

        i_GraphicsPipeline &operator=(const i_GraphicsPipeline &other);

        void BeginRenderPassAndBindPipeline(VkCommandBuffer commandBuffer);

        void EndRenderPass(VkCommandBuffer commandBuffer);

        VkPipelineLayout GetPipelineLayout();

        static VkRenderPass GetRenderPass();

        static VkFramebuffer GetFramebuffer(uint32_t index);

    private:
        void Destruct();

        static void CreateRenderPass();

        static void CreateFramebuffers();

        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        static VkRenderPass renderPass;
        static std::vector<VkFramebuffer> framebuffers;

        static std::mutex renderPassMutex;
        std::unique_ptr<std::lock_guard<std::mutex>> renderPassLock;

        std::shared_ptr<uint32_t> useCount;
    };
}

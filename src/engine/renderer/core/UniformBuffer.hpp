#pragma once


#include <vector>
#include <memory>
#include <list>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "Shader.hpp"
#include "Semaphore.hpp"

#define UNIFORM_BUFFER_COMMAND_BUFFERS_PER_THREAD 1

namespace renderer {
    struct i_UniformBufferCreateInfo {
        uint32_t binding;
        uint32_t threadIndex;

        std::vector<ShaderHandle > shaders;

        void *data;
        size_t size;

        i_Semaphore signalSemaphore;
        uint64_t signalSemaphoreValue;
    };


    class i_UniformBuffer {
    public:
        static void Update();

        i_UniformBuffer();

        i_UniformBuffer(const i_UniformBufferCreateInfo &createInfo);

        i_UniformBuffer(const i_UniformBuffer &other);

        i_UniformBuffer &operator=(const i_UniformBuffer &other);

        ~i_UniformBuffer();

        void SetBinding(uint32_t binding);

        void UpdateData(void *data, size_t size, uint32_t threadIndex);

        void SetSignalSemaphoreValue(uint64_t value);

        void Destructor();

    private:
        i_DataBuffer dataBuffer;

        size_t size;
        uint32_t binding;
        std::vector<ShaderHandle > shaders;

        std::shared_ptr<uint32_t> useCount;

        static std::vector<VkWriteDescriptorSet> writeDescriptorSetsQueue;
        static std::list<VkDescriptorBufferInfo> bufferInfoList;
    };
}

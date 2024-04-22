#pragma once

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace renderer{

class Shader{
public:
    Shader(const char* vertexShaderPath, const char* fragmentShaderPath);
    ~Shader();
    Shader(const Shader& other);
    Shader operator=(const Shader& other);

    VkShaderModule GetVertexShaderModule() const;
    VkShaderModule GetFragmentShaderModule() const;

    std::array<VkPipelineShaderStageCreateInfo, 2> GetShaderStageCreateInfo() const;
private:
    std::vector<unsigned char> ReadBytecode(const char* path); 

    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;

    uint32_t* useCount;
};

}
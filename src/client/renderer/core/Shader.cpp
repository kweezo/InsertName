#include "Shader.hpp"

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath){
    useCount = new uint32_t;
    useCount[0] = 1;

    std::vector<unsigned char> vertexShaderBytecode = ReadBytecode(vertexShaderPath);
    std::vector<unsigned char> fragmentShaderBytecode = ReadBytecode(fragmentShaderPath);

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = vertexShaderBytecode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(vertexShaderBytecode.data());

    if(vkCreateShaderModule(Device::GetDevice(), &createInfo, nullptr, &vertexShaderModule) != VK_SUCCESS){
        throw std::runtime_error("Failed to create vertex shader module");
    }

    createInfo.codeSize = fragmentShaderBytecode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentShaderBytecode.data());

    if(vkCreateShaderModule(Device::GetDevice(), &createInfo, nullptr, &fragmentShaderModule) != VK_SUCCESS){
        throw std::runtime_error("Failed to create fragment shader module");
    }
}

std::vector<unsigned char> Shader::ReadBytecode(const char* path){
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if(!file.is_open()){
        std::string error = "Failed to open file: " + std::string(path);
        throw std::runtime_error(path);
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<unsigned char> buffer(fileSize);

    file.seekg(0);

    file.read((char*) buffer.data(), fileSize);

    return buffer;

}

std::array<VkPipelineShaderStageCreateInfo, 2> Shader::GetShaderStageCreateInfo() const{
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertexShaderModule;
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragmentShaderModule;
    fragmentShaderStageInfo.pName = "main";

    return {vertexShaderStageInfo, fragmentShaderStageInfo};
}

Shader::Shader(const Shader& other){
    useCount = other.useCount;
    useCount[0]++;
    vertexShaderModule = other.vertexShaderModule;
    fragmentShaderModule = other.fragmentShaderModule;
}

Shader Shader::operator=(const Shader& other){
    if(this == &other){
        return *this;
    }

    useCount = other.useCount;
    useCount[0]++;
    vertexShaderModule = other.vertexShaderModule;
    fragmentShaderModule = other.fragmentShaderModule;
    return *this;
}

Shader::~Shader(){
    if(useCount[0] <= 1){
        vkDestroyShaderModule(Device::GetDevice(), vertexShaderModule, nullptr);
        vkDestroyShaderModule(Device::GetDevice(), fragmentShaderModule, nullptr);
        delete[] useCount;
    }
    else{
        useCount[0]--;
    }
}
#include "Shader.hpp"

namespace renderer{

std::vector<ShaderBindingInfo> ShaderImpl::shaderBindings = {};


ShaderHandle Shader::CreateShader(const char* vertexShaderPath, const char* fragmentShaderPath,
 std::vector<VkDescriptorSetLayoutBinding> bindings){
    return new ShaderImpl(vertexShaderPath, fragmentShaderPath, bindings);
}

void Shader::Free(ShaderHandle shader){
    delete shader;
}

void Shader::EnableNewShaders(){
    ShaderImpl::EnableNewShaders();
}

void ShaderImpl::EnableNewShaders(){
    std::vector<VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutInfos;

    VkDescriptorType descriptorTypes{};

    for(ShaderBindingInfo& bindingInfo : shaderBindings){
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = bindingInfo.bindings.size();
        descriptorSetLayoutInfo.pBindings = bindingInfo.bindings.data();

        descriptorSetLayoutInfos.push_back(descriptorSetLayoutInfo);
    }

    uint32_t layoutIndex = DescriptorManager::CreateLayouts(descriptorSetLayoutInfos)[0];
    
    std::vector<DescriptorBatchInfo> batchInfos;
    uint32_t i = 0;
    for(VkDescriptorSetLayoutBinding& binding : shaderBindings[i].bindings){
        batchInfos.push_back({1, binding.descriptorType});
    }

    std::vector<DescriptorHandle> handles = DescriptorManager::CreateDescriptors(batchInfos, shaderBindings.size(), layoutIndex);

    for(uint32_t i = 0; i < handles.size(); i++){
        shaderBindings[i].handle->SetDescriptorSet(DescriptorManager::GetDescriptorSet(handles[i]));
    }
}

void ShaderImpl::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout){
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
}

void ShaderImpl::SetDescriptorSet(VkDescriptorSet descriptorSet){
    this->descriptorSet = descriptorSet;
}


VkDescriptorSet ShaderImpl::GetDescriptorSet() const{
    return descriptorSet;
}

ShaderImpl::ShaderImpl(const char* vertexShaderPath, const char* fragmentShaderPath, std::vector<VkDescriptorSetLayoutBinding> bindings){
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

    shaderBindings.push_back({this, bindings});
}

std::vector<unsigned char> ShaderImpl::ReadBytecode(const char* path){
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

void ShaderImpl::UpdateDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets){
    vkUpdateDescriptorSets(Device::GetDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

std::array<VkPipelineShaderStageCreateInfo, 2> ShaderImpl::GetShaderStageCreateInfo() const{
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

ShaderImpl::ShaderImpl(const ShaderImpl& other){
    useCount = other.useCount;
    useCount[0]++;
    vertexShaderModule = other.vertexShaderModule;
    fragmentShaderModule = other.fragmentShaderModule;
    descriptorSet = other.descriptorSet;
}

ShaderImpl ShaderImpl::operator=(const ShaderImpl& other){
    if(this == &other){
        return *this;
    }

    useCount = other.useCount;
    useCount[0]++;
    vertexShaderModule = other.vertexShaderModule;
    fragmentShaderModule = other.fragmentShaderModule;
    descriptorSet = other.descriptorSet;
    return *this;
}

ShaderImpl::~ShaderImpl(){
    if(useCount[0] <= 1){
        vkDestroyShaderModule(Device::GetDevice(), vertexShaderModule, nullptr);
        vkDestroyShaderModule(Device::GetDevice(), fragmentShaderModule, nullptr);
        delete[] useCount;
    }
    else{
        useCount[0]--;
    }
}

}
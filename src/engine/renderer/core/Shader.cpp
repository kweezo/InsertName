#include "Shader.hpp"

namespace renderer{

const std::string shaderPath = "client_data/shaders/bin/";

std::vector<ShaderBindingInfo> ShaderImpl::shaderBindings = {};
std::unordered_map<std::string, ShaderHandle> Shader::shaders = {};

void Shader::Initialize(){
    std::ifstream stream("client_data/shaders/shaders.json");
    if(!stream.is_open()){
        throw std::runtime_error("Failed to open shaders.json");
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string err;
    if(!Json::parseFromStream(builder, stream, &root, &err)){
        throw std::runtime_error("Failed to parse shaders.json: " + err);
    }


    const Json::Value& shaders = root["shaders"];
    for(const Json::Value& shader : shaders){
        std::vector<VkDescriptorSetLayoutBinding> descriptorBindings;
        
        std::string vertexPath = shader["vertexPath"].asString();
        std::string fragmentPath = shader["fragmentPath"].asString();
        std::string name = shader["name"].asString();

        const Json::Value& bindings = shader["bindings"];
        for (const Json::Value& binding : bindings){
            int bindingIndex = binding["binding"].asInt();
            std::string type = binding["type"].asString();
            std::string stage = binding["stage"].asString();

            VkDescriptorSetLayoutBinding descriptorBinding{};
            descriptorBinding.binding = bindingIndex;
            descriptorBinding.descriptorCount = 1;
            
            if(!type.compare("uniform_buffer")){
                descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            }
            else if(!type.compare("sampler")){
                descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            }
            else{
                throw std::runtime_error("Error when reading shaders.json, invalid descriptor type");
            }

            if(!stage.compare("vertex")){
                descriptorBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            }
            else if(!stage.compare("fragment")){
                descriptorBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            }
            else{
                throw std::runtime_error("Error when reading shaders.json, invalid shader stage");
            }

            descriptorBindings.push_back(descriptorBinding);

        }

        vertexPath = shaderPath + vertexPath;
        fragmentPath = shaderPath + fragmentPath;

        Shader::shaders[name] = new ShaderImpl(vertexPath.c_str(), fragmentPath.c_str(),
        name.c_str(), descriptorBindings);
    }

    ShaderImpl::EnableNewShaders();

    stream.close();
}

ShaderHandle Shader::GetShader(std::string name){
    if(shaders.find(name) == shaders.end()){
        throw std::runtime_error("Attempting to get nonexistent shader with name " + name);
    }
    return shaders[name];
}

void Shader::Cleanup(){
    for(auto& [name, shader] : shaders){
        delete shader;
    }
}


void ShaderImpl::EnableNewShaders(){
    if(shaderBindings.empty()){
        std::cout << "Warning, tried to enable new shader bindings but no new shaders have been created" << std::endl;
        return;
    }

    std::vector<VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutInfos;

    VkDescriptorType descriptorTypes{};

    ShaderBindingInfo& firstBindingInfo = shaderBindings[0];

    for(uint32_t i = 1; i < shaderBindings.size(); i++){
        firstBindingInfo.bindings.insert(firstBindingInfo.bindings.end(), shaderBindings[i].bindings.begin(), shaderBindings[i].bindings.end());
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = firstBindingInfo.bindings.size();
    descriptorSetLayoutInfo.pBindings = firstBindingInfo.bindings.data();
    descriptorSetLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV;

    uint32_t layoutIndex = DescriptorManager::CreateLayouts(descriptorSetLayoutInfo);
    
    std::unordered_map<VkDescriptorType, uint32_t> bindingCounts;

    std::vector<DescriptorBatchInfo> batchInfos;
    uint32_t i = 0;
    for(VkDescriptorSetLayoutBinding& binding : shaderBindings[i].bindings){
        if(bindingCounts.find(binding.descriptorType) == bindingCounts.end()){
            bindingCounts[binding.descriptorType] = 1;
        }
        else{
            bindingCounts[binding.descriptorType]++;
        }
    }

    for(auto& [descriptorType, count] : bindingCounts){
        batchInfos.push_back({count, descriptorType});
    }

    for(uint32_t y = 0; y < shaderBindings.size(); y++){
        std::vector<DescriptorHandle> handles = DescriptorManager::CreateDescriptors(batchInfos, shaderBindings[y].bindings.size(), layoutIndex);

        for(uint32_t i = 0; i < handles.size(); i++){
            shaderBindings[i].handle->SetDescriptorSet(DescriptorManager::GetDescriptorSet(handles[i]));
        }
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

ShaderImpl::ShaderImpl(const char* vertexShaderPath, const char* fragmentShaderPath, const char* name,
 std::vector<VkDescriptorSetLayoutBinding> bindings){
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
    this->name = name;
}

std::vector<unsigned char> ShaderImpl::ReadBytecode(const char* path){
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if(!file.is_open()){
        std::string error = "Failed to open file: " + std::string(path);
        throw std::runtime_error(error);
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

const char* ShaderImpl::GetName(){
    return name;
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
#include "Shader.hpp"

namespace renderer{

const std::string shaderPath = "client_data/shaders/bin/";

std::vector<_ShaderBindingInfo> _Shader::shaderBindings = {};
boost::container::flat_map<std::string, std::shared_ptr<_Shader>> _ShaderManager::shaders = {};

void _ShaderManager::Init(){
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

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};

        const Json::Value& inputBindings = shader["bindingDescriptions"];
        for(const Json::Value& inputBinding : inputBindings){
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = inputBinding["binding"].asInt();
            bindingDescription.stride = inputBinding["stride"].asInt();
            bindingDescription.inputRate = (VkVertexInputRate)inputBinding["inputRate"].asInt();

            bindingDescriptions.push_back(bindingDescription);
        }
        
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        
        const Json::Value& inputAttributes = shader["inputAttributes"];
        for(const Json::Value& inputAttribute : inputAttributes){
            VkVertexInputAttributeDescription attributeDescription{};
            attributeDescription.binding = inputAttribute["binding"].asInt();
            attributeDescription.format = (VkFormat)inputAttribute["format"].asInt();
            attributeDescription.location = inputAttribute["location"].asInt();
            attributeDescription.offset = inputAttribute["offset"].asInt();

            attributeDescriptions.push_back(attributeDescription);
        }

        vertexPath = shaderPath + vertexPath;
        fragmentPath = shaderPath + fragmentPath;

        __VertexInputDescriptions inputDescriptions = {attributeDescriptions, bindingDescriptions};

        _ShaderManager::shaders.emplace(name, std::make_shared<_Shader>(vertexPath, fragmentPath,
        name, descriptorBindings, inputDescriptions));
    }

    _Shader::CreateDescriptorSets();

    for(auto& [name, shader] : _ShaderManager::shaders){
        shader->CreateGraphicsPipepeline();
    }

    stream.close();
}

std::shared_ptr<_Shader> _ShaderManager::GetShader(std::string name){
    if(shaders.find(name) == shaders.end()){
        throw std::runtime_error("Attempting to get nonexistent shader with name " + name);
    }
    return shaders[name];
}

void _ShaderManager::Cleanup(){
    shaders.clear();
}


void _Shader::CreateDescriptorSets(){
    if(shaderBindings.empty()){
        std::cout << "Warning, tried to enable new shader bindings but no new shaders have been created" << std::endl;
        return;
    }

    std::vector<VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutInfos(shaderBindings.size());



    _DescriptorSetBatchAllocateInfo allocInfo{}; 
    
    allocInfo.descriptorLayoutBindings.resize(shaderBindings.size());
    for(uint32_t i = 0; i < shaderBindings.size(); i++){
        allocInfo.descriptorLayoutBindings[i]= shaderBindings[i].bindings;
    }


    std::vector<_DescriptorSetLocation> locations = _DescriptorManager::AllocateDescriptorSetBatch(allocInfo);
    

    for(uint32_t i = 0; i < locations.size(); i++){
        shaderBindings[i].handle->SetDescriptorSet(_DescriptorManager::RetrieveDescriptorSet(locations[i]));
    }

}

void _Shader::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout){
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
}

void _Shader::SetDescriptorSet(VkDescriptorSet descriptorSet){
    this->descriptorSet = descriptorSet;
}

_GraphicsPipeline* _Shader::GetGraphicsPipeline(){
    return &graphicsPipeline;
}

VkDescriptorSet _Shader::GetDescriptorSet() const{
    return descriptorSet;
}

void _Shader::CreateGraphicsPipepeline(){
    _GraphicsPipelineCreateInfo createInfo{};
    createInfo.shaderStageCreateInfo = GetShaderStageCreateInfo();
    createInfo.attributeDescriptions = std::get<0>(vertexInputDescriptions);
    createInfo.bindingDescriptions = std::get<1>(vertexInputDescriptions);
    
    graphicsPipeline = _GraphicsPipeline(createInfo);
}

_Shader::_Shader(){
}

_Shader::_Shader(const std::string vertexShaderPath, const std::string fragmentShaderPath, const std::string name, 
     std::vector<VkDescriptorSetLayoutBinding> bindings, __VertexInputDescriptions vertexInputDescriptions){    

    useCount = std::make_shared<uint32_t>(1);

    std::vector<unsigned char> vertexShaderBytecode = ReadBytecode(vertexShaderPath);
    std::vector<unsigned char> fragmentShaderBytecode = ReadBytecode(fragmentShaderPath);

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = vertexShaderBytecode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(vertexShaderBytecode.data());

    if(vkCreateShaderModule(_Device::GetDevice(), &createInfo, nullptr, &vertexShaderModule) != VK_SUCCESS){
        throw std::runtime_error("Failed to create vertex shader module");
    }

    createInfo.codeSize = fragmentShaderBytecode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentShaderBytecode.data());

    if(vkCreateShaderModule(_Device::GetDevice(), &createInfo, nullptr, &fragmentShaderModule) != VK_SUCCESS){
        throw std::runtime_error("Failed to create fragment shader module");
    }

    this->vertexInputDescriptions = vertexInputDescriptions;

    shaderBindings.push_back({this, bindings});
    this->name = name;
}

std::vector<unsigned char> _Shader::ReadBytecode(const std::string path){
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if(!file.is_open()){
        std::string error = "Failed to open file: " + path;
        throw std::runtime_error(error);
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<unsigned char> buffer(fileSize);

    file.seekg(0);
    file.read((char*) buffer.data(), fileSize);

    return buffer;

}

void _Shader::UpdateDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets){
    vkUpdateDescriptorSets(_Device::GetDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

const std::string _Shader::GetName(){
    return name;
}

std::array<VkPipelineShaderStageCreateInfo, 2> _Shader::GetShaderStageCreateInfo() const{
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


_Shader::~_Shader(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount.get() <= 1){
        vkDestroyShaderModule(_Device::GetDevice(), vertexShaderModule, nullptr);
        vkDestroyShaderModule(_Device::GetDevice(), fragmentShaderModule, nullptr);
        
        useCount.reset();

        return;
    }

    (*useCount.get())--;
}

}
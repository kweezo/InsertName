#include "Shader.hpp"

namespace renderer{

const std::string shaderPath = "client_data/shaders/bin/";

std::vector<__ShaderBindingInfo> __Shader::shaderBindings = {};
boost::container::flat_map<std::string, std::shared_ptr<__Shader>> __ShaderManager::shaders = {};

void __ShaderManager::Init(){
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

        __ShaderManager::shaders.emplace(name, std::make_shared<__Shader>(vertexPath, fragmentPath,
        name, descriptorBindings));
    }

    __Shader::CreateDescriptorSets();

    stream.close();
}

std::shared_ptr<__Shader> __ShaderManager::GetShader(std::string name){
    if(shaders.find(name) == shaders.end()){
        throw std::runtime_error("Attempting to get nonexistent shader with name " + name);
    }
    return shaders[name];
}

void __ShaderManager::Cleanup(){
    shaders.clear();
}


void __Shader::CreateDescriptorSets(){
    if(shaderBindings.empty()){
        std::cout << "Warning, tried to enable new shader bindings but no new shaders have been created" << std::endl;
        return;
    }

    std::vector<VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutInfos(shaderBindings.size());



    __DescriptorSetBatchAllocateInfo allocInfo{}; 
    
    allocInfo.descriptorLayoutBindings.resize(shaderBindings.size());
    for(uint32_t i = 0; i < shaderBindings.size(); i++){
        allocInfo.descriptorLayoutBindings[i]= shaderBindings[i].bindings;
    }


    std::vector<__DescriptorSetLocation> locations = __DescriptorManager::AllocateDescriptorSetBatch(allocInfo);
    

    for(uint32_t i = 0; i < locations.size(); i++){
        shaderBindings[i].handle->SetDescriptorSet(__DescriptorManager::RetrieveDescriptorSet(locations[i]));
    }

}

void __Shader::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout){
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
}

void __Shader::SetDescriptorSet(VkDescriptorSet descriptorSet){
    this->descriptorSet = descriptorSet;
}

__GraphicsPipeline* __Shader::GetGraphicsPipeline(){
    return &graphicsPipeline;
}

VkDescriptorSet __Shader::GetDescriptorSet() const{
    return descriptorSet;
}

void __Shader::CreateGraphicsPipepeline(__VertexInputDescriptions vertexInputDescriptions){
    __GraphicsPipelineCreateInfo createInfo{};
    createInfo.shaderStageCreateInfo = GetShaderStageCreateInfo();
    createInfo.attributeDescriptions = std::get<0>(vertexInputDescriptions);
    createInfo.bindingDescriptions = std::get<1>(vertexInputDescriptions);
    
    graphicsPipeline = __GraphicsPipeline(createInfo);
}

__Shader::__Shader(){
    useCount = std::make_shared<uint32_t>(1);
}

__Shader::__Shader(const std::string vertexShaderPath, const std::string fragmentShaderPath, const std::string name, 
     std::vector<VkDescriptorSetLayoutBinding> bindings){    

    useCount = std::make_shared<uint32_t>(1);

    std::vector<unsigned char> vertexShaderBytecode = ReadBytecode(vertexShaderPath);
    std::vector<unsigned char> fragmentShaderBytecode = ReadBytecode(fragmentShaderPath);

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = vertexShaderBytecode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(vertexShaderBytecode.data());

    if(vkCreateShaderModule(__Device::GetDevice(), &createInfo, nullptr, &vertexShaderModule) != VK_SUCCESS){
        throw std::runtime_error("Failed to create vertex shader module");
    }

    createInfo.codeSize = fragmentShaderBytecode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentShaderBytecode.data());

    if(vkCreateShaderModule(__Device::GetDevice(), &createInfo, nullptr, &fragmentShaderModule) != VK_SUCCESS){
        throw std::runtime_error("Failed to create fragment shader module");
    }

    shaderBindings.push_back({this, bindings});
    this->name = name;
}

std::vector<unsigned char> __Shader::ReadBytecode(const std::string path){
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

void __Shader::UpdateDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets){
    vkUpdateDescriptorSets(__Device::GetDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

const std::string __Shader::GetName(){
    return name;
}

std::array<VkPipelineShaderStageCreateInfo, 2> __Shader::GetShaderStageCreateInfo() const{
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

__Shader::__Shader(const __Shader& other){
    useCount = other.useCount;
    vertexShaderModule = other.vertexShaderModule;
    fragmentShaderModule = other.fragmentShaderModule;
    descriptorSet = other.descriptorSet;
    name = other.name;

    (*useCount.get())++;
}

__Shader __Shader::operator=(const __Shader& other){
    if(this == &other){
        return *this;
    }

    useCount = other.useCount;
    vertexShaderModule = other.vertexShaderModule;
    fragmentShaderModule = other.fragmentShaderModule;
    descriptorSet = other.descriptorSet;
    name = other.name;

    (*useCount.get())++;

    return *this;
}

__Shader::~__Shader(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount.get() <= 1){
        vkDestroyShaderModule(__Device::GetDevice(), vertexShaderModule, nullptr);
        vkDestroyShaderModule(__Device::GetDevice(), fragmentShaderModule, nullptr);
        
        useCount.reset();

        return;
    }

    (*useCount.get())--;
}

}
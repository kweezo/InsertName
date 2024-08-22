#include "instance.hpp"


const std::string instanceConfigPath = "engine_data/config/instance.json";

namespace renderer{

    std::unique_ptr<i_Instance> i_Instance::instance;

    void i_Instance::Create(std::string appName, uint32_t version){
        instance.reset(new i_Instance(appName, version));
    }

    void i_Instance::Destroy(){
        instance.reset();
    }

    i_Instance::i_Instance(std::string appName, uint32_t version){
        GetRequiredExtensions();
        LoadConfig();
        CreateVulkanInstance(appName, version);
    }


    i_Instance::~i_Instance(){
        vkDestroyInstance(vulkanInstance, nullptr);
    }

    void i_Instance::GetRequiredExtensions(){

        uint32_t requiredExtensionCount;
        glfwGetRequiredInstanceExtensions(&requiredExtensionCount);

        extensions.resize(requiredExtensionCount);
        memcpy(extensions.data(), glfwGetRequiredInstanceExtensions(&requiredExtensionCount), requiredExtensionCount);

        

    }

    void i_Instance::LoadConfig(){
        std::ifstream stream(instanceConfigPath);
        if(!stream.is_open()){
            throw std::runtime_error("ERROR: Unable to open instance config");
        }

        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string error;

        if(!Json::parseFromStream(builder, stream, &root, &error)){
            throw std::runtime_error("ERROR: Failed to parse shaders.json: " + error);
        }


        const Json::Value& layers = root["layers"];
        for(const Json::Value& layer : layers){
            this->layers.push_back(layer.asCString());
        }


        const Json::Value& extensions = root["extensions"];
        for(const Json::Value& extension : extensions){
            this->extensions.push_back(extension.asCString());
        }
    

        stream.close();
    }



    void i_Instance::CreateVulkanInstance(std::string appName, uint32_t version){
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

        appInfo.apiVersion = VK_MAKE_API_VERSION(1, 3, 0, 0);
        appInfo.applicationVersion = version;
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);

        appInfo.pEngineName = "sugondeznuts";
        appInfo.pApplicationName = appName.c_str();


        VkInstanceCreateInfo instanceInfo{};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    #ifdef NDEBUG
        instanceInfo.enabledLayerCount = layers.size();
        instanceInfo.ppEnabledLayerNames = layers.data();
    #else
        instanceInfo.enabledLayerCount = 0;
    #endif
    }

}
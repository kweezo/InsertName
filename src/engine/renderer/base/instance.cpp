#include "instance.hpp"


const std::string instanceConfigPath = "engine_data/config/instance.json";


VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    if(messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT){
        std::string message = "ERROR: ";
        message.append(pCallbackData->pMessage);
        throw std::runtime_error(message);
    }

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

namespace renderer{

    std::unique_ptr<i_Instance> i_Instance::instance;

    void i_Instance::Create(std::string appName, uint32_t version){
        instance.reset(new i_Instance(appName, version));
    }

    void i_Instance::Destroy(){
        instance.reset();
    }

    i_Instance::i_Instance(std::string appName, uint32_t version){
        if(glfwInit() == GLFW_FALSE){// because I Have to ok
            throw std::runtime_error("ERROR: Failed to init GLFW");
        }

        GetRequiredExtensions();
        LoadConfig();
        CreateVulkanInstance(appName, version);
#ifdef NDEBUG
        SetupDebugMessenger();
#endif
    }


    i_Instance::~i_Instance(){  
        extensions.erase(extensions.begin(), extensions.begin() + glfwExtensionCount);
        for(char* ext : extensions){
            delete[] ext;
        }

        for(char* layer : layers){
            delete[] layer;
        }

#ifdef NDEBUG
        DestroyDebugUtilsMessengerEXT(vulkanInstance, debugMessenger, nullptr);
#endif
        vkDestroyInstance(vulkanInstance, nullptr);
    }

    void i_Instance::GetRequiredExtensions(){

        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        extensions.resize(glfwExtensionCount);
        memcpy(extensions.data(), glfwGetRequiredInstanceExtensions(&glfwExtensionCount), glfwExtensionCount * sizeof(char*));

        

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
            const char* buff = layer.asCString();
            this->layers.push_back(new char[strlen(buff)+1]);

            strcpy(this->layers.back(), buff);
        }


        const Json::Value& extensions_required = root["extensions_required"];
        for(const Json::Value& extension : extensions_required){
            const char* buff = extension.asCString();
            this->extensions.push_back(new char[strlen(buff)+1]);

            strcpy(this->extensions.back(), buff);
        }

        std::list<uint32_t> unsupportedExtensionIndexes = CheckExtensionSupport(extensions);
        if(!unsupportedExtensionIndexes.empty()){
            std::cerr << "ERROR: Not all required extensions are supported: \n";
            for(uint32_t i : unsupportedExtensionIndexes){
                std::cerr << "\t" << extensions[i] << "\n"; 
            }
            throw std::runtime_error("Program is terminating");
        }


        const Json::Value& extensions_optional = root["extensions_optional"];
        for(const Json::Value& extension : extensions_optional){
            const char* buff = extension.asCString();
            this->supportedOptionalExtensions.push_back(new char[strlen(buff)+1]);

            strcpy(this->supportedOptionalExtensions.back(), buff);
        }

        unsupportedExtensionIndexes = CheckExtensionSupport(supportedOptionalExtensions);
        if(!unsupportedExtensionIndexes.empty()){
            uint32_t erasedExtCount = 0;
            std::cout << "INFO: Not all optional extensions are supported: \n";
            for(uint32_t i : unsupportedExtensionIndexes){
                std::cout << "\t" << supportedOptionalExtensions[i-erasedExtCount] << "\n"; 
                supportedOptionalExtensions.erase(supportedOptionalExtensions.begin() + i - erasedExtCount);

                erasedExtCount++;
            }
        } 
        
        extensions.insert(extensions.begin(), supportedOptionalExtensions.begin(), supportedOptionalExtensions.end());

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

        VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
        debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugMessengerInfo.pfnUserCallback = debugCallback;

        instanceInfo.pNext = &debugMessengerInfo;
    #else
        instanceInfo.enabledLayerCount = 0;
    #endif

        instanceInfo.enabledExtensionCount = extensions.size();
        instanceInfo.ppEnabledExtensionNames = extensions.data();

        if(vkCreateInstance(&instanceInfo, nullptr, &vulkanInstance) != VK_SUCCESS){
            throw std::runtime_error("ERROR: Failed to create a Vulkan instance");
        }
    }


    void i_Instance::SetupDebugMessenger(){
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;

        if(CreateDebugUtilsMessengerEXT(vulkanInstance, &createInfo, nullptr, &debugMessenger)){
            std::cerr << "WARNING: Failed to setup the debug messenger\n";
        }
    }
    
    VkInstance i_Instance::GetInstance(){
        return instance->vulkanInstance;
    }
    
    
    std::list<uint32_t> i_Instance::CheckExtensionSupport(const std::vector<char*>& extensions){
        std::list<uint32_t> unsupportedExtensionIndexes;

        uint32_t propertyCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);

        std::vector<VkExtensionProperties> properties(propertyCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, properties.data());


        for(uint32_t i = 0; i < extensions.size(); i++){
            bool match = false;
            for(uint32_t y = 0; y < properties.size(); y++){
                if(strcmp(extensions[i], properties[y].extensionName) == 0){
                    match = true;
                    break;
                }
            }

            if(match){
                continue;
            }

            unsupportedExtensionIndexes.push_back(i);
        }
        
        return unsupportedExtensionIndexes;
    }

    const std::vector<char*>& i_Instance::GetSupportedOptionalExtensions(){
        return instance->supportedOptionalExtensions;
    }



}
#include "Instance.hpp"

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
   // "VK_LAYER_LUNARG_api_dump",
#ifdef GFX_RECONSTRUCT
    //"VK_LAYER_LUNARG_gfxreconstruct",
#endif
};

std::vector<const char*> instanceExtensions = {
#ifdef NDEBUG
    "VK_EXT_debug_utils",
#endif


};

namespace renderer{

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void OnGpuCrashDump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* userData){
    std::ofstream out(GPU_CRASH_LOG_FILE, std::ios::out | std::ios::binary);

    if(!out.is_open()){
        throw std::runtime_error("Failed to open file for gpu dump");
    }
    out.write(static_cast<const char*>(pGpuCrashDump), gpuCrashDumpSize);
    out.close();
}

void OnShaderDebugInfo(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void* pUserData){

}

/*void OnDescription(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription, void* userData){

}*/

void OnResolveMarker(const void* pMarkerData, const uint32_t markerDataSize, void* pUserData, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize){

}


VkInstance i_Instance::instance = VK_NULL_HANDLE;
VkDebugUtilsMessengerEXT i_Instance::debugMessenger = VK_NULL_HANDLE;

void i_Instance::Init(){
#ifdef NDEBUG
 /*   GFSDK_Aftermath_Result result = GFSDK_Aftermath_EnableGpuCrashDumps(
        GFSDK_Aftermath_Version_API,
        GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan,
        GFSDK_Aftermath_GpuCrashDumpFeatureFlags_Default,
        OnGpuCrashDump,
        OnShaderDebugInfo,
        OnDescription,
        OnResolveMarker,
        nullptr
    );*/
#endif

    if (instance != VK_NULL_HANDLE) {
        throw std::runtime_error("Vulkan instance already created");
    }
    if(!Window::GetGLFWInitialized()){
        throw std::runtime_error("GLFW not initialized");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Client";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Niko je weeb";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 3, 0);

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);;

    for(int i = 0; i < glfwExtensionCount; i++){
        instanceExtensions.push_back(glfwExtensions[i]);
    }

    createInfo.enabledExtensionCount = instanceExtensions.size();
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

#ifdef NDEBUG
    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();
#else
    createInfo.enabledLayerCount = 0;
#endif

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }

#ifdef NDEBUG
    SetupDebugMessenger();
#endif
}

void i_Instance::SetupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug messenger");
    }
}

VkInstance i_Instance::GetInstance(){
    if (instance == VK_NULL_HANDLE) {
        throw std::runtime_error("Vulkan instance not created");
    }
    return instance;
}

void i_Instance::Cleanup(){
    if (debugMessenger != VK_NULL_HANDLE) {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr) {
            func(instance, debugMessenger, nullptr);
        }
    }

    if (instance == VK_NULL_HANDLE) {
        throw std::runtime_error("Vulkan instance not created");
    }
    vkDestroyInstance(instance, nullptr);
}

}
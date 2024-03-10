#include "Instance.hpp"

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

VkInstance Instance::instance = VK_NULL_HANDLE;

void Instance::CreateInstance(){
    if (instance != VK_NULL_HANDLE) {
        throw std::runtime_error("Vulkan instance already created");
    }
    if(!Window::GetGlfwInitialized()){
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
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    
    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();

    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

VkInstance Instance::GetInstance(){
    if (instance == VK_NULL_HANDLE) {
        throw std::runtime_error("Vulkan instance not created");
    }
    return instance;
}

void Instance::DestroyInstance(){
    if (instance == VK_NULL_HANDLE) {
        throw std::runtime_error("Vulkan instance not created");
    }
    vkDestroyInstance(instance, nullptr);
}
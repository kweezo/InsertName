#include "Device.hpp"

VkDevice Device::device = VK_NULL_HANDLE;
VkPhysicalDevice Device::physicalDevice = VK_NULL_HANDLE;

void Device::CreateDevice(){
    PickPhysicalDevice();
}

void Device::PickPhysicalDevice(){
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(Instance::GetInstance(), &deviceCount, nullptr);

    if(deviceCount == 0){
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(Instance::GetInstance(), &deviceCount, devices.data());

    physicalDevice = devices[0];

    for(const auto& currDevice : devices){
        if(currDevice == VK_NULL_HANDLE){
            continue;
        }

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(currDevice, &deviceProperties);

        if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            physicalDevice = currDevice;
            break;
        }
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    std::cout << "Using device: " << deviceProperties.deviceName << std::endl;

    if(physicalDevice == VK_NULL_HANDLE){
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

void Device::DestroyDevice(){
    vkDestroyDevice(device, nullptr);
}
#include "Device.hpp"

VkDevice Device::device = VK_NULL_HANDLE;
VkPhysicalDevice Device::physicalDevice = VK_NULL_HANDLE;
QueueFamilyInfo Device::queueFamilyInfo = {};

void Device::CreateDevice(){
    PickPhysicalDevice();
    CreateQueueCreateInfos();
    CreateLogicalDevice();
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

void Device::CreateLogicalDevice(){
    if(queueFamilyInfo.graphicsFamilyFound == false){
        throw std::runtime_error("Failed to find a suitable queue family!");
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    std::vector<float> graphicsQueuePriorities(queueFamilyInfo.graphicsQueueCreateInfo.queueCount, 1.0f);
    queueFamilyInfo.graphicsQueueCreateInfo.pQueuePriorities = graphicsQueuePriorities.data();

    queueCreateInfos.push_back(queueFamilyInfo.graphicsQueueCreateInfo);

    if(queueFamilyInfo.transferFamilyFound){
        std::vector<float> transferQueuePriorities(queueFamilyInfo.transferQueueCreateInfo.queueCount, 1.0f);
        queueFamilyInfo.transferQueueCreateInfo.pQueuePriorities = transferQueuePriorities.data();
        queueCreateInfos.push_back(queueFamilyInfo.transferQueueCreateInfo);
    }


    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount = 0;

    if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS){
        throw std::runtime_error("Failed to create logical device!");
    }

}

void Device::CreateQueueCreateInfos(){
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
    
    for(int i = 0; i < (int)queueFamilies.size(); i++){
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            queueFamilyInfo.graphicsFamilyFound = true;
            queueFamilyInfo.graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueFamilyInfo.graphicsQueueCreateInfo.queueFamilyIndex = i;   
            queueFamilyInfo.graphicsQueueCreateInfo.queueCount = queueFamilies[i].queueCount;
        }
        else if(queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT){
            queueFamilyInfo.transferFamilyFound = true;
            queueFamilyInfo.transferQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueFamilyInfo.transferQueueCreateInfo.queueFamilyIndex = i;   
            queueFamilyInfo.transferQueueCreateInfo.queueCount = queueFamilies[i].queueCount;
        }
    }
}

VkPhysicalDevice Device::GetPhysicalDevice(){
    return physicalDevice;
}
VkDevice Device::GetDevice(){
    return device;
}

void Device::DestroyDevice(){
    vkDestroyDevice(device, nullptr);
}
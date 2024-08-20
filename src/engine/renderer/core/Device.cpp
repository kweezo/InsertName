#include "Device.hpp"

namespace renderer {
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData) {
        if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            return VK_FALSE;
        }

        std::cerr << pCallbackData->pMessage << std::endl;

        static const char *MEMORY_ERR = "VK_ERROR_OUT_OF_DEVICE_MEMORY";

        if (!strcmp(pCallbackData->pMessage, MEMORY_ERR)) {
            i_Device::SetDeviceMemoryFull();
        }

        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            throw std::runtime_error(
                "Validation error too severe, aborting. Please write me with the error message (jakob.tadej.vrtacnik@students.vegova.si).");
        }

        return VK_FALSE;
    }

    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        "VK_EXT_descriptor_indexing"
    };

    VkDevice i_Device::device = VK_NULL_HANDLE;
    VkPhysicalDevice i_Device::physicalDevice = VK_NULL_HANDLE;
    i_QueueFamilyInfo i_Device::queueFamilyInfo = {};
    std::vector<VkQueue> i_Device::graphicsQueues = {};
    std::vector<VkQueue> i_Device::transferQueues = {};
    uint32_t i_Device::graphicsQueueFamilyIndex = 0;
    uint32_t i_Device::transferQueueFamilyIndex = 0;
    bool i_Device::deviceMemoryFree = false;
    VkPhysicalDeviceProperties i_Device::physicalDeviceProperties = {};
    bool i_Device::initialized = false;

    void i_Device::Init() {
        PickPhysicalDevice();
        CreateQueueCreateInfos();
        CreateLogicalDevice();
        GetQueues();
        initialized = true;
    }

    void i_Device::PickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(i_Instance::GetInstance(), &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(i_Instance::GetInstance(), &deviceCount, devices.data());

        physicalDevice = devices[0];

        for (const auto &currDevice: devices) {
            if (currDevice == VK_NULL_HANDLE) {
                continue;
            }

            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(currDevice, &deviceProperties);

            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                physicalDevice = currDevice;
                deviceMemoryFree = true;
                break;
            }
        }

        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        std::cout << "Using device: " << physicalDeviceProperties.deviceName << std::endl;

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }

    VkPhysicalDeviceFeatures i_Device::GetAvailableDeviceFeatures() {
        VkPhysicalDeviceFeatures availableFeatures{};
        vkGetPhysicalDeviceFeatures(physicalDevice, &availableFeatures);


        VkPhysicalDeviceFeatures wantedFeatures{};

        wantedFeatures.samplerAnisotropy = VK_TRUE;

        for (uint32_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); i++) {
            if ((reinterpret_cast<VkBool32 *>(&wantedFeatures))[i] == VK_TRUE && (reinterpret_cast<VkBool32 *>(&
                    availableFeatures))[i] == VK_FALSE) {
                throw std::runtime_error("Device does not support required features!");
            }
        }

        return wantedFeatures;
    }

    bool i_Device::DeviceMemoryFree() {
        return deviceMemoryFree;
    }

    void i_Device::SetDeviceMemoryFull() {
        deviceMemoryFree = false;
    }

    VkPhysicalDeviceProperties i_Device::GetPhysicalDeviceProperties() {
        return physicalDeviceProperties;
    }

    void i_Device::CreateLogicalDevice() {
        if (queueFamilyInfo.graphicsFamilyFound == false) {
            throw std::runtime_error("Failed to find a suitable queue family!");
        }

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        const std::vector<float> graphicsQueuePriorities(queueFamilyInfo.graphicsQueueCreateInfo.queueCount, 1.0f);
        queueFamilyInfo.graphicsQueueCreateInfo.pQueuePriorities = graphicsQueuePriorities.data();

        queueCreateInfos.push_back(queueFamilyInfo.graphicsQueueCreateInfo);

        std::vector<float> transferQueuePriorities(queueFamilyInfo.transferQueueCreateInfo.queueCount, 1.0f);
        if (queueFamilyInfo.transferFamilyFound) {
            queueFamilyInfo.transferQueueCreateInfo.pQueuePriorities = transferQueuePriorities.data();
            queueCreateInfos.push_back(queueFamilyInfo.transferQueueCreateInfo);
        }

        VkPhysicalDeviceVulkan12Features vulkan12Features{};
        vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        vulkan12Features.timelineSemaphore = VK_TRUE;
        vulkan12Features.descriptorIndexing = VK_TRUE;

        VkPhysicalDeviceFeatures2 deviceFeatures{};
        deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures.pNext = &vulkan12Features;
        deviceFeatures.features.samplerAnisotropy = VK_TRUE;


        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &deviceFeatures;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = deviceExtensions.size();
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        createInfo.enabledLayerCount = 0;

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }
    }

    void i_Device::CreateQueueCreateInfos() {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        for (int i = 0; i < static_cast<int>(queueFamilies.size()); i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamilyInfo.graphicsFamilyFound = true;
                queueFamilyInfo.graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueFamilyInfo.graphicsQueueCreateInfo.queueFamilyIndex = i;
                queueFamilyInfo.graphicsQueueCreateInfo.queueCount = queueFamilies[i].queueCount;
            } else if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
                queueFamilyInfo.transferFamilyFound = true;
                queueFamilyInfo.transferQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueFamilyInfo.transferQueueCreateInfo.queueFamilyIndex = i;
                queueFamilyInfo.transferQueueCreateInfo.queueCount = queueFamilies[i].queueCount;
            }
        }
    }

    void i_Device::GetQueues() {
        for (int i = 0; i < queueFamilyInfo.graphicsQueueCreateInfo.queueCount; i++) {
            VkQueue queue;
            vkGetDeviceQueue(device, queueFamilyInfo.graphicsQueueCreateInfo.queueFamilyIndex, i, &queue);
            graphicsQueues.push_back(queue);
        }

        if (queueFamilyInfo.transferFamilyFound) {
            for (int i = 0; i < queueFamilyInfo.transferQueueCreateInfo.queueCount; i++) {
                VkQueue queue;
                vkGetDeviceQueue(device, queueFamilyInfo.transferQueueCreateInfo.queueFamilyIndex, i, &queue);
                transferQueues.push_back(queue);
            }
        }
    }

    VkQueue i_Device::GetGraphicsQueue() {
        VkQueue &queue = graphicsQueues[graphicsQueueFamilyIndex];
        graphicsQueueFamilyIndex = (graphicsQueueFamilyIndex + 1) % graphicsQueues.size();

        return queue;
    }

    VkQueue i_Device::GetTransferQueue() {
        if (queueFamilyInfo.transferFamilyFound == false) {
            return GetGraphicsQueue();
        }

        VkQueue &queue = transferQueues[transferQueueFamilyIndex];
        transferQueueFamilyIndex = (transferQueueFamilyIndex + 1) % transferQueues.size();

        return queue;
    }

    i_QueueFamilyInfo i_Device::GetQueueFamilyInfo() {
        return queueFamilyInfo;
    }

    VkPhysicalDevice i_Device::GetPhysicalDevice() {
        return physicalDevice;
    }

    VkDevice i_Device::GetDevice() {
        return device;
    }

    bool i_Device::IsInitialized() {
        return initialized;
    }

    void i_Device::Cleanup() {
        vkDestroyDevice(device, nullptr);
    }
}

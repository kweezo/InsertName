#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "Instance.hpp"

namespace renderer{

typedef struct i_QueueFamilyInfo{
    bool graphicsFamilyFound = false;
    bool transferFamilyFound = false;


    VkDeviceQueueCreateInfo graphicsQueueCreateInfo{};
    VkDeviceQueueCreateInfo transferQueueCreateInfo{};
} i_QueueFamilyInfo;


class i_Device{
public:
    static void Init();
    static void Cleanup();

    static VkDevice GetDevice();
    static VkPhysicalDevice GetPhysicalDevice();

    static VkQueue GetGraphicsQueue();
    static VkQueue GetTransferQueue();

    static i_QueueFamilyInfo GetQueueFamilyInfo();
    static VkPhysicalDeviceProperties GetPhysicalDeviceProperties();

    static bool DeviceMemoryFree();
    static void SetDeviceMemoryFull();

    static bool IsInitialized();
private:
    static VkPhysicalDeviceFeatures GetAvailableDeviceFeatures();

    static uint32_t graphicsQueueFamilyIndex;
    static uint32_t transferQueueFamilyIndex;

    static std::vector<VkQueue> graphicsQueues;
    static std::vector<VkQueue> transferQueues;

    static VkDevice device;
    static VkPhysicalDevice physicalDevice;

    static VkPhysicalDeviceProperties physicalDeviceProperties;

    static bool deviceMemoryFree;
    static bool initialized;

    static i_QueueFamilyInfo queueFamilyInfo;

    static void PickPhysicalDevice();
    static void CreateLogicalDevice();
    static void CreateQueueCreateInfos();
    static void GetQueues();

};

}
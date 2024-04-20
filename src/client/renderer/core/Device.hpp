#pragma once

#include <stdexcept>
#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#include "Instance.hpp"

namespace renderer{

typedef struct QueueFamilyInfo{
    bool graphicsFamilyFound = false;
    bool transferFamilyFound = false;


    VkDeviceQueueCreateInfo graphicsQueueCreateInfo{};
    VkDeviceQueueCreateInfo transferQueueCreateInfo{};
} QueueFamilyInfo;


class Device{
public:
    static void CreateDevice();
    static void DestroyDevice();

    static VkDevice GetDevice();
    static VkPhysicalDevice GetPhysicalDevice();

    static VkQueue GetGraphicsQueue();
    static VkQueue GetTransferQueue();

    static QueueFamilyInfo GetQueueFamilyInfo();

    static bool DeviceMemoryFree();
    static void SetDeviceMemoryFull();
private:
    static uint32_t graphicsQueueFamilyIndex;
    static uint32_t transferQueueFamilyIndex;

    static std::vector<VkQueue> graphicsQueues;
    static std::vector<VkQueue> transferQueues;

    static VkDevice device;
    static VkPhysicalDevice physicalDevice;

    static bool deviceMemoryFree;

    static QueueFamilyInfo queueFamilyInfo;

    static void PickPhysicalDevice();
    static void CreateLogicalDevice();
    static void CreateQueueCreateInfos();
    static void GetQueues();

};

}
#pragma once

#include <stdexcept>
#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#include "Instance.hpp"

typedef struct QueueFamilyInfo{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyFound = false;
    bool presentFamilyFound = false;

    uint32_t graphicsQueueCount;
    uint32_t transferQueueCount;

    VkDeviceQueueCreateInfo graphicsQueueCreateInfo;
    VkDeviceQueueCreateInfo transferQueueCreateInfo;
} QueueFamilyInfo;


class Device{
public:
    static void CreateDevice();
    static void DestroyDevice();

    static VkDevice GetDevice();
private:
    static VkDevice device;
    static VkPhysicalDevice physicalDevice;

    static void PickPhysicalDevice();
    static void PickLogicalDevice();
    static void CreateQueueCreateInfos();

};

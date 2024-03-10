#pragma once

#include <stdexcept>
#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#include "Instance.hpp"

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
private:
    static VkDevice device;
    static VkPhysicalDevice physicalDevice;

    static QueueFamilyInfo queueFamilyInfo;

    static void PickPhysicalDevice();
    static void CreateLogicalDevice();
    static void CreateQueueCreateInfos();

};

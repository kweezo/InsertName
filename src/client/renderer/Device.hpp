#pragma once

#include <stdexcept>
#include <iostream>

#include <vulkan/vulkan.h>

#include "Instance.hpp"


class Device{
public:
    static void CreateDevice();
    static void DestroyDevice();

    static VkDevice GetDevice();
private:
    static VkDevice device;
    static VkPhysicalDevice physicalDevice;

    static void PickPhysicalDevice();
};

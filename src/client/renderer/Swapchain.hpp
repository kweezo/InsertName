#pragma once

#include <stdexcept>
#include <algorithm>

#include <vulkan/vulkan.h>

#include "../window/Window.hpp"
#include "Device.hpp"

#define PREFERRED_IMAGE_COUNT (uint32_t)3

class Swapchain{
public:
    static void CreateSwapchain();
    static void DestroySwapchain();

    static VkSwapchainKHR GetSwapchain();
private:
    static VkSwapchainKHR swapchain;
    static std::vector<VkImageView> swapchainImageViews;

    static VkFormat ChooseSwapchainImageFormat();

    static void CreateSwapchainImageViews();
};

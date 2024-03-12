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

    static VkExtent2D GetExtent();
    static VkFormat GetImageFormat();

    static std::vector<VkImageView> GetSwapchainImageViews();
private:
    static VkSwapchainKHR swapchain;
    static std::vector<VkImageView> swapchainImageViews;

    std::vector<VkFramebuffer> framebuffers;

    static VkFormat ChooseSwapchainImageFormat();

    static void CreateSwapchainImageViews();
};

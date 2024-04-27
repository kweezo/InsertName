#pragma once

#include <stdexcept>
#include <algorithm>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#include "../window/Window.hpp"
#include "Device.hpp"
#include "Image.hpp"

#define PREFERRED_IMAGE_COUNT (uint32_t)3

namespace renderer{

class Swapchain{
public:
    static void CreateSwapchain();
    static void DestroySwapchain();

    static VkSwapchainKHR GetSwapchain();

    static VkExtent2D GetExtent();
    static VkFormat GetImageFormat();

    static uint32_t GetImageCount();

    static std::vector<VkImageView> GetSwapchainImageViews();
private:
    static VkSwapchainKHR swapchain;
    static std::vector<VkImageView> swapchainImageViews;
    static ImageHandle depthImage;

    static void CreateDepthImage();


    static VkFormat ChooseSwapchainImageFormat();
    static void CreateSwapchainImageViews();
};

}
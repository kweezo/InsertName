#pragma once

#include <stdexcept>
#include <algorithm>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#include "../window/Window.hpp"
#include "Device.hpp"
#include "Image.hpp"

namespace renderer{

class Swapchain{
public:
    static void Init();
    static void Cleanup();

    static VkSwapchainKHR GetSwapchain();
    static VkExtent2D GetExtent();
    static VkFormat GetImageFormat();
    static VkFormat GetDepthFormat();
    static uint32_t GetImageCount();
    static Image GetDepthImage();


    static std::vector<VkImageView> GetSwapchainImageViews();
private:
    static VkSwapchainKHR swapchain;
    static std::vector<VkImageView> swapchainImageViews;

    static Image depthImage;
    static VkFormat depthFormat;

    static void CreateDepthImage();

    static VkFormat ChooseSwapchainImageFormat();
    static void CreateSwapchainImageViews();
};

}
#pragma once

#include <stdexcept>
#include <algorithm>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#include "../window/Window.hpp"
#include "Device.hpp"
#include "Image.hpp"

#define MAX_FRAMES_IN_FLIGHT 2

namespace renderer{

class __Swapchain{
public:
    static void Init();
    static void Cleanup();

    static VkSwapchainKHR GetSwapchain();
    static VkExtent2D GetExtent();
    static VkFormat GetImageFormat();
    static VkFormat GetDepthFormat();
    static uint32_t GetImageCount();
    static __Image GetDepthImage();


    static std::vector<VkImageView> GetSwapchainImageViews();
private:
    static VkSwapchainKHR swapchain;
    static std::vector<VkImageView> swapchainImageViews;

    static __Image depthImage;
    static VkFormat depthFormat;

    static void CreateDepthImage();

    static VkFormat ChooseSwapchainImageFormat();
    static void CreateSwapchainImageViews();
};

}
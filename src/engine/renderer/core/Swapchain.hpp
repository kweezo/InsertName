#pragma once

#include <stdexcept>
#include <algorithm>
#include <limits>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#include "../window/Window.hpp"
#include "Device.hpp"
#include "Image.hpp"
#include "Semaphore.hpp"

#define MAX_FRAMES_IN_FLIGHT 2

namespace renderer{

class _Swapchain{
public:
    static void Init();
    static void Cleanup();

    static VkSwapchainKHR GetSwapchain();
    static VkExtent2D GetExtent();
    static VkFormat GetImageFormat();
    static VkFormat GetDepthFormat();
    static uint32_t GetImageCount();
    static _Image GetDepthImage();

    static void IncrementCurrentFrameInFlight();
    static void IncrementCurrentFrameIndex(_Semaphore semaphore);

    static uint32_t GetFrameInFlight();
    static uint32_t GetImageIndex();

    static std::vector<VkImageView> GetSwapchainImageViews();
private:
    static VkSwapchainKHR swapchain;
    static std::vector<VkImageView> swapchainImageViews;

    static _Image depthImage;
    static VkFormat depthFormat;

    static void CreateDepthImage();

    static VkFormat ChooseSwapchainImageFormat();
    static void CreateSwapchainImageViews();

    static uint32_t currentImageIndex;
    static uint32_t currentFrameInFlight;
};

}
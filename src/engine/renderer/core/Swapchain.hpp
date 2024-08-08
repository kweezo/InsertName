#pragma once

#include <stdexcept>
#include <algorithm>
#include <limits>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#include "../window/Window.hpp"
#include "Device.hpp"
#include "Semaphore.hpp"
#include "Image.hpp"


namespace renderer{

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

class i_Swapchain{
public:
    static void Init();
    static void Cleanup();

    static VkSwapchainKHR GetSwapchain();
    static VkExtent2D GetExtent();
    static VkFormat GetImageFormat();
    static VkFormat GetDepthFormat();
    static uint32_t GetImageCount();
    static i_Image GetDepthImage();

    static void IncrementCurrentFrameInFlight();
    static void IncrementCurrentFrameIndex(const i_Semaphore& semaphore);

    static uint32_t GetFrameInFlight();
    static uint32_t GetImageIndex();

    static std::vector<VkImageView> GetSwapchainImageViews();
private:
    static VkSwapchainKHR swapchain;
    static std::vector<VkImageView> swapchainImageViews;

    static i_Image depthImage;
    static VkFormat depthFormat;

    static void CreateDepthImage();

    static VkFormat ChooseSwapchainImageFormat();
    static void CreateSwapchainImageViews();

    static uint32_t currentImageIndex;
    static uint32_t currentFrameInFlight;
};

}
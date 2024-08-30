#pragma once

#include <exception>
#include <memory>
#include <algorithm>

#include <vulkan/vulkan.h>

#include "window.hpp"
#include "physicalDevice.hpp"
#include "logicalDevice.hpp"

namespace renderer{

class i_Swapchain{
public:
    static void Create();
    static void Destroy();

    ~i_Swapchain();
private:
    i_Swapchain();
    i_Swapchain(const i_Swapchain& other) = delete;
    i_Swapchain& operator=(const i_Swapchain& other) = delete;

    static std::unique_ptr<i_Swapchain> swapchain;

    void CreateSwapchain();

    VkSwapchainCreateInfoKHR GetSwapchainCreateInfo();

    const uint32_t prefferedImageCount = 3;

    VkSwapchainKHR vulkanSwapchain;

};

}

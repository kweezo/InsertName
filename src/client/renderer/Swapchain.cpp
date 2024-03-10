#include "Swapchain.hpp"

VkSwapchainKHR Swapchain::swapchain = VK_NULL_HANDLE;

void Swapchain::CreateSwapchain(){
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device::GetPhysicalDevice(), Window::GetVulkanSurface(),
     &surfaceCapabilities);
   
    VkSwapchainCreateInfoKHR createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = Window::GetVulkanSurface();
    createInfo.minImageCount = std::min(surfaceCapabilities.minImageCount, PREFERRED_IMAGE_COUNT);
    createInfo.imageFormat = ChooseSwapchainImageFormat();
}

VkFormat Swapchain::ChooseSwapchainImageFormat(){
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device::GetPhysicalDevice(), Window::GetVulkanSurface(), &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    for(const auto& format : formats){
        if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            return format.format;
        }
    }
    return formats[0].format;
}

void Swapchain::DestroySwapchain(){
    vkDestroySwapchainKHR(Device::GetDevice(), swapchain, nullptr);
}

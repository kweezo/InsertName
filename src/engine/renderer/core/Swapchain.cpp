#include "Swapchain.hpp"

namespace renderer{

//TODO allow window resizing

VkSwapchainKHR Swapchain::swapchain = VK_NULL_HANDLE;
std::vector<VkImageView> Swapchain::swapchainImageViews = {};
ImageHandle Swapchain::depthImage = nullptr;

void Swapchain::CreateSwapchain(){
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device::GetPhysicalDevice(), Window::GetVulkanSurface(),
     &surfaceCapabilities);

   
    VkSwapchainCreateInfoKHR createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = Window::GetVulkanSurface();
    if(surfaceCapabilities.maxImageCount == 0){ //no upper bound
        createInfo.minImageCount = std::max(PREFERRED_IMAGE_COUNT, surfaceCapabilities.minImageCount);
    }
    else{
        createInfo.minImageCount = (std::min)(surfaceCapabilities.maxImageCount, PREFERRED_IMAGE_COUNT);
    }
    createInfo.imageFormat = ChooseSwapchainImageFormat();
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    int width, height;
    glfwGetWindowSize(Window::GetGLFWwindow(), &width, &height);

    createInfo.imageExtent.width = std::clamp((uint32_t)width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
    createInfo.imageExtent.height = std::clamp((uint32_t)height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if(Device::GetQueueFamilyInfo().transferFamilyFound){
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        uint32_t indices[] = {Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex,
         Device::GetQueueFamilyInfo().transferQueueCreateInfo.queueFamilyIndex};

        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    }
    else{
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
    }

    createInfo.preTransform = surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = swapchain;

    if(vkCreateSwapchainKHR(Device::GetDevice(), &createInfo, nullptr, &swapchain) != VK_SUCCESS){
        throw std::runtime_error("Failed to create swapchain!");
    }

    CreateSwapchainImageViews();
    CreateDepthImage();
}

VkFormat Swapchain::ChooseSwapchainImageFormat(){
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device::GetPhysicalDevice(), Window::GetVulkanSurface(), &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device::GetPhysicalDevice(), Window::GetVulkanSurface(), &formatCount, formats.data());
    for(const auto& format : formats){
        if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            return format.format;
        }
    }
    return formats[0].format;
}

void Swapchain::CreateSwapchainImageViews(){
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(Device::GetDevice(), swapchain, &imageCount, nullptr);
    std::vector<VkImage> swapchainImages(imageCount);
    vkGetSwapchainImagesKHR(Device::GetDevice(), swapchain, &imageCount, swapchainImages.data());

    swapchainImageViews.resize(imageCount);
    for(size_t i = 0; i < imageCount; i++){
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = ChooseSwapchainImageFormat();
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(Device::GetDevice(), &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create image views!");
        }
    }
}

void Swapchain::CreateDepthImage(){
    VkFormat depthFormat = Image::GetSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
     VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    depthImage = Image::CreateImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, GetExtent().width, GetExtent().height, 0, nullptr);

    depthImage->TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    
}

VkSwapchainKHR Swapchain::GetSwapchain(){
    return swapchain;
}

VkExtent2D Swapchain::GetExtent(){
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device::GetPhysicalDevice(), Window::GetVulkanSurface(),
     &surfaceCapabilities);
    return surfaceCapabilities.currentExtent;
}

uint32_t Swapchain::GetImageCount(){
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(Device::GetDevice(), swapchain, &imageCount, nullptr);
    return imageCount;
}

std::vector<VkImageView> Swapchain::GetSwapchainImageViews(){
    return swapchainImageViews;
}

VkFormat Swapchain::GetImageFormat(){
    return ChooseSwapchainImageFormat();
}

void Swapchain::DestroySwapchain(){
    for(auto imageView : swapchainImageViews){
        vkDestroyImageView(Device::GetDevice(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(Device::GetDevice(), swapchain, nullptr);
}

}
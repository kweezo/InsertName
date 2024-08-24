#include "Swapchain.hpp"

namespace renderer{

//TODO allow window resizing

const uint32_t PREFERRED_IMAGE_COUNT = 2;
uint32_t _Swapchain::currentImageIndex{};
uint32_t _Swapchain::currentFrameInFlight{};

VkSwapchainKHR _Swapchain::swapchain = VK_NULL_HANDLE;
std::vector<VkImageView> _Swapchain::swapchainImageViews = {};
_Image _Swapchain::depthImage = {};
VkFormat _Swapchain::depthFormat = {};

void _Swapchain::Init(){
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_Device::GetPhysicalDevice(), Window::GetVulkanSurface(),
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

    if(_Device::GetQueueFamilyInfo().transferFamilyFound){
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        uint32_t indices[] = {_Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex,
         _Device::GetQueueFamilyInfo().transferQueueCreateInfo.queueFamilyIndex};

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

    if(vkCreateSwapchainKHR(_Device::GetDevice(), &createInfo, nullptr, &swapchain) != VK_SUCCESS){
        throw std::runtime_error("Failed to create swapchain!");
    }

    CreateSwapchainImageViews();
    CreateDepthImage();
}

VkFormat _Swapchain::ChooseSwapchainImageFormat(){
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_Device::GetPhysicalDevice(), Window::GetVulkanSurface(), &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(_Device::GetPhysicalDevice(), Window::GetVulkanSurface(), &formatCount, formats.data());
    for(const auto& format : formats){
        if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            return format.format;
        }
    }
    return formats[0].format;
}

void _Swapchain::CreateSwapchainImageViews(){
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(_Device::GetDevice(), swapchain, &imageCount, nullptr);
    std::vector<VkImage> swapchainImages(imageCount);
    vkGetSwapchainImagesKHR(_Device::GetDevice(), swapchain, &imageCount, swapchainImages.data());

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

        if(vkCreateImageView(_Device::GetDevice(), &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create image views!");
        }
    }
}

VkFormat _Swapchain::GetDepthFormat(){
    return depthFormat;
}

_Image _Swapchain::GetDepthImage(){
    return depthImage;
}

void _Swapchain::CreateDepthImage(){
    VkSurfaceCapabilitiesKHR surfaceCapabilites{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_Device::GetPhysicalDevice(), Window::GetVulkanSurface(), &surfaceCapabilites);


    depthFormat = _Image::GetSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
     VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);


    _ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.format = depthFormat;
    imageCreateInfo.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageCreateInfo.imageExtent =
     {std::clamp(GetExtent().width, surfaceCapabilites.minImageExtent.width, surfaceCapabilites.maxImageExtent.width),
    std::clamp(GetExtent().height, surfaceCapabilites.minImageExtent.height, surfaceCapabilites.maxImageExtent.height)};
    imageCreateInfo.data = (void*)0;
    imageCreateInfo.size = 0;
    imageCreateInfo.threadIndex = 0;
    imageCreateInfo.copyToLocalDeviceMemory = true;

    depthImage = _Image(imageCreateInfo);

    depthImage.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkSwapchainKHR _Swapchain::GetSwapchain(){
    return swapchain;
}

VkExtent2D _Swapchain::GetExtent(){
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_Device::GetPhysicalDevice(), Window::GetVulkanSurface(),
     &surfaceCapabilities);
    return surfaceCapabilities.currentExtent;
}

uint32_t _Swapchain::GetImageCount(){
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(_Device::GetDevice(), swapchain, &imageCount, nullptr);
    return imageCount;
}

std::vector<VkImageView> _Swapchain::GetSwapchainImageViews(){
    return swapchainImageViews;
}
void _Swapchain::IncrementCurrentFrameInFlight(){
    currentFrameInFlight += (currentFrameInFlight + 1) % MAX_FRAMES_IN_FLIGHT;
}

void _Swapchain::IncrementCurrentFrameIndex(_Semaphore semaphore){
    vkAcquireNextImageKHR(_Device::GetDevice(), swapchain, std::numeric_limits<uint64_t>::max(), semaphore.GetSemaphore(), VK_NULL_HANDLE, &currentImageIndex);
}

uint32_t _Swapchain::GetFrameInFlight(){
    return currentFrameInFlight;
}

uint32_t _Swapchain::GetImageIndex(){
    return currentImageIndex;
}

VkFormat _Swapchain::GetImageFormat(){
    return ChooseSwapchainImageFormat();
}

void _Swapchain::Cleanup(){
    for(auto imageView : swapchainImageViews){
        vkDestroyImageView(_Device::GetDevice(), imageView, nullptr);
    }
    vkDestroySwapchainKHR(_Device::GetDevice(), swapchain, nullptr);

    depthImage.~_Image();
}

}
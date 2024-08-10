#include "Swapchain.hpp"

#include <glm/ext/scalar_uint_sized.hpp>

namespace renderer {
    //TODO allow window resizing

    constexpr uint32_t PREFERRED_IMAGE_COUNT = 2;
    uint32_t i_Swapchain::currentImageIndex{};
    uint32_t i_Swapchain::currentFrameInFlight{};

    VkSwapchainKHR i_Swapchain::swapchain = VK_NULL_HANDLE;
    std::vector<VkImageView> i_Swapchain::swapchainImageViews = {};
    i_Image i_Swapchain::depthImage = {};
    VkFormat i_Swapchain::depthFormat = {};

    void i_Swapchain::Init() {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(i_Device::GetPhysicalDevice(), Window::GetVulkanSurface(),
                                                  &surfaceCapabilities);


        VkSwapchainCreateInfoKHR createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = Window::GetVulkanSurface();
        if (surfaceCapabilities.maxImageCount == 0) {
            //no upper bound
            createInfo.minImageCount = std::max(PREFERRED_IMAGE_COUNT, surfaceCapabilities.minImageCount);
        } else {
            createInfo.minImageCount = (std::min)(surfaceCapabilities.maxImageCount, PREFERRED_IMAGE_COUNT);
        }
        createInfo.imageFormat = ChooseSwapchainImageFormat();
        createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        int width, height;
        glfwGetWindowSize(Window::GetGLFWwindow(), &width, &height);

        createInfo.imageExtent.width = std::clamp(static_cast<uint32_t>(width),
                                                  surfaceCapabilities.minImageExtent.width,
                                                  surfaceCapabilities.maxImageExtent.width);
        createInfo.imageExtent.height = std::clamp(static_cast<uint32_t>(height),
                                                   surfaceCapabilities.minImageExtent.height,
                                                   surfaceCapabilities.maxImageExtent.height);
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if (i_Device::GetQueueFamilyInfo().transferFamilyFound) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            uint32_t indices[] = {
                i_Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex,
                i_Device::GetQueueFamilyInfo().transferQueueCreateInfo.queueFamilyIndex
            };

            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = indices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
        }

        createInfo.preTransform = surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = swapchain;

        if (vkCreateSwapchainKHR(i_Device::GetDevice(), &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain!");
        }

        CreateSwapchainImageViews();
        CreateDepthImage();
    }

    VkFormat i_Swapchain::ChooseSwapchainImageFormat() {
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(i_Device::GetPhysicalDevice(), Window::GetVulkanSurface(), &formatCount,
                                             nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(i_Device::GetPhysicalDevice(), Window::GetVulkanSurface(), &formatCount,
                                             formats.data());
        for (const auto &format: formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format.format;
            }
        }
        return formats[0].format;
    }

    void i_Swapchain::CreateSwapchainImageViews() {
        uint32_t imageCount;
        vkGetSwapchainImagesKHR(i_Device::GetDevice(), swapchain, &imageCount, nullptr);
        std::vector<VkImage> swapchainImages(imageCount);
        vkGetSwapchainImagesKHR(i_Device::GetDevice(), swapchain, &imageCount, swapchainImages.data());

        swapchainImageViews.resize(imageCount);
        for (size_t i = 0; i < imageCount; i++) {
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

            if (vkCreateImageView(i_Device::GetDevice(), &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image views!");
            }
        }
    }

    VkFormat i_Swapchain::GetDepthFormat() {
        return depthFormat;
    }

    i_Image i_Swapchain::GetDepthImage() {
        return depthImage;
    }

    void i_Swapchain::CreateDepthImage() {
        VkSurfaceCapabilitiesKHR surfaceCapabilites{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(i_Device::GetPhysicalDevice(), Window::GetVulkanSurface(),
                                                  &surfaceCapabilites);


        depthFormat = i_Image::GetSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);


        i_ImageCreateInfo imageCreateInfo{};
        imageCreateInfo.format = depthFormat;
        imageCreateInfo.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageCreateInfo.imageExtent =
        {
            std::clamp(GetExtent().width, surfaceCapabilites.minImageExtent.width,
                       surfaceCapabilites.maxImageExtent.width),
            std::clamp(GetExtent().height, surfaceCapabilites.minImageExtent.height,
                       surfaceCapabilites.maxImageExtent.height)
        };
        imageCreateInfo.data = nullptr;
        imageCreateInfo.size = 0;
        imageCreateInfo.threadIndex = 0;
        imageCreateInfo.copyToLocalDeviceMemory = true;

        depthImage = i_Image(imageCreateInfo);

        depthImage.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    VkSwapchainKHR i_Swapchain::GetSwapchain() {
        return swapchain;
    }

    VkExtent2D i_Swapchain::GetExtent() {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(i_Device::GetPhysicalDevice(), Window::GetVulkanSurface(),
                                                  &surfaceCapabilities);
        return surfaceCapabilities.currentExtent;
    }

    uint32_t i_Swapchain::GetImageCount() {
        uint32_t imageCount;
        vkGetSwapchainImagesKHR(i_Device::GetDevice(), swapchain, &imageCount, nullptr);
        return imageCount;
    }

    std::vector<VkImageView> i_Swapchain::GetSwapchainImageViews() {
        return swapchainImageViews;
    }

    void i_Swapchain::IncrementCurrentFrameInFlight() {
        currentFrameInFlight += (currentFrameInFlight + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void i_Swapchain::IncrementCurrentFrameIndex(const i_Semaphore &semaphore) {
        vkAcquireNextImageKHR(i_Device::GetDevice(), swapchain, std::numeric_limits<uint64_t>::max(),
                              semaphore.GetSemaphore(), VK_NULL_HANDLE, &currentImageIndex);
    }

    uint32_t i_Swapchain::GetFrameInFlight() {
        return currentFrameInFlight;
    }

    uint32_t i_Swapchain::GetImageIndex() {
        return currentImageIndex;
    }

    VkFormat i_Swapchain::GetImageFormat() {
        return ChooseSwapchainImageFormat();
    }

    void i_Swapchain::Cleanup() {
        for (auto imageView: swapchainImageViews) {
            vkDestroyImageView(i_Device::GetDevice(), imageView, nullptr);
        }
        vkDestroySwapchainKHR(i_Device::GetDevice(), swapchain, nullptr);

        depthImage.Destruct();
    }
}

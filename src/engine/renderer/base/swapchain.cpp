#include "swapchain.hpp"


namespace renderer{
    std::unique_ptr<i_Swapchain> i_Swapchain::swapchain;

    void i_Swapchain::Create(){
        swapchain.reset(new i_Swapchain());
    }

    void i_Swapchain::Destroy(){
        swapchain.reset();
    }


    i_Swapchain::i_Swapchain(){
        CreateSwapchain();
    }

    void i_Swapchain::CreateSwapchain(){
        VkSwapchainCreateInfoKHR createInfo{};

        createInfo = GetSwapchainCreateInfo();

        if(vkCreateSwapchainKHR(i_LogicalDevice::GetDevice(), &createInfo, nullptr, &vulkanSwapchain) != VK_SUCCESS){
            throw std::runtime_error("ERROR: Failed to re/create the swapchain");
        }
    }

    VkSwapchainCreateInfoKHR i_Swapchain::GetSwapchainCreateInfo(){
        VkSurfaceCapabilitiesKHR surfaceCapabilites;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(i_PhysicalDevice::GetDevice(), i_Window::GetSurface(), &surfaceCapabilites);

        VkExtent2D currentExtent  = {
            std::clamp(surfaceCapabilites.currentExtent.width, surfaceCapabilites.minImageExtent.width, surfaceCapabilites.maxImageExtent.width),
            std::clamp(surfaceCapabilites.currentExtent.height, surfaceCapabilites.minImageExtent.height, surfaceCapabilites.maxImageExtent.height)
        };

        uint32_t imageCount;
        if(surfaceCapabilites.maxImageCount == 0){
            imageCount = std::max(prefferedImageCount, surfaceCapabilites.minImageCount);
        }else{
            imageCount = std::clamp(prefferedImageCount, surfaceCapabilites.minImageCount, surfaceCapabilites.maxImageCount);
        }

        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(i_PhysicalDevice::GetDevice(), i_Window::GetSurface(), &surfaceFormatCount, nullptr);

        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(i_PhysicalDevice::GetDevice(), i_Window::GetSurface(), &surfaceFormatCount, surfaceFormats.data());


        VkSurfaceFormatKHR swapchainFormat{};

        bool formatFound = false;
        for(const VkSurfaceFormatKHR& format : surfaceFormats){
            if(format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_R8G8B8A8_SRGB){
                swapchainFormat = format;
                formatFound = true;
                break;
            }
        }

        if(!formatFound){
            swapchainFormat = surfaceFormats.front();
        }


        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = i_Window::GetSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageArrayLayers = 1;
        createInfo.imageFormat = swapchainFormat.format;
        createInfo.imageExtent = currentExtent;
        createInfo.imageColorSpace = swapchainFormat.colorSpace;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        createInfo.clipped = VK_TRUE;
        //TODO createInfo.oldSwapchain = vulkanSwapchain;

        return createInfo;
    }

    i_Swapchain::~i_Swapchain(){
        vkDestroySwapchainKHR(i_LogicalDevice::GetDevice(), vulkanSwapchain, nullptr);
    }

}
#include "Renderer.hpp"

namespace renderer{

void Renderer::InitRenderer(){
    Instance::CreateInstance();
    Device::CreateDevice();
    Window::CreateVulkanSurface();
    Swapchain::CreateSwapchain();
    CommandPool::CreateCommandPools();
    ImageImpl::Initialize();
}

void Renderer::RenderFrame(){
    DataBuffer::UpdateCommandBuffer();
}

void Renderer::DestroyRenderer(){
    Swapchain::DestroySwapchain();
    DescriptorManager::Cleanup();
    DataBuffer::Cleanup();
    CommandPool::DestroyCommandPools();
    vkDestroySurfaceKHR(Instance::GetInstance(), Window::GetVulkanSurface(), nullptr);
    Device::DestroyDevice();
    Instance::DestroyInstance();
}
}
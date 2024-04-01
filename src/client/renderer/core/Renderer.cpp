#include "Renderer.hpp"

namespace renderer{

void Renderer::InitRenderer(){
    Instance::CreateInstance();
    Device::CreateDevice();
    Window::CreateVulkanSurface();
    Swapchain::CreateSwapchain();
    CommandPool::CreateCommandPools();
}

void Renderer::RenderFrame(){
    DataBuffer::UpdateCommandBuffer();
}

void Renderer::DestroyRenderer(){
    CommandPool::DestroyCommandPools();
    Swapchain::DestroySwapchain();
    DataBuffer::Cleanup();
    vkDestroySurfaceKHR(Instance::GetInstance(), Window::GetVulkanSurface(), nullptr);
    Device::DestroyDevice();
    Instance::DestroyInstance();
}
}
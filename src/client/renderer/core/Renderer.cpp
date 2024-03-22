#include "Renderer.hpp"


void Renderer::InitRenderer(){
    Instance::CreateInstance();
    Device::CreateDevice();
    Window::CreateVulkanSurface();
    Swapchain::CreateSwapchain();
    CommandPool::CreateCommandPools();
}

void Renderer::RenderFrame(){
    VertexBuffer::UpdateCommandBuffer();
}

void Renderer::DestroyRenderer(){
    CommandPool::DestroyCommandPools();
    Swapchain::DestroySwapchain();
    vkDestroySurfaceKHR(Instance::GetInstance(), Window::GetVulkanSurface(), nullptr);
    Device::DestroyDevice();
    Instance::DestroyInstance();
}
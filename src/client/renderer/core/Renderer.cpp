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
    VertexBuffer::UpdateCommandBuffer();
}

void Renderer::DestroyRenderer(){
    Swapchain::DestroySwapchain();
    VertexBuffer::Cleanup();
    CommandPool::DestroyCommandPools();
    vkDestroySurfaceKHR(Instance::GetInstance(), Window::GetVulkanSurface(), nullptr);
    Device::DestroyDevice();
    Instance::DestroyInstance();
}

}
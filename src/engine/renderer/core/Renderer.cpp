#include "Renderer.hpp"

namespace renderer{

void Renderer::InitRenderer(){
    HardInit();
    SoftInit();
}
void Renderer::HardInit(){
    Instance::CreateInstance();
    Device::CreateDevice();
    Window::CreateVulkanSurface();
    CommandPool::CreateCommandPools();
    ImageImpl::Initialize();
}

void Renderer::SoftInit(){
    Swapchain::CreateSwapchain();
    Shader::Initialize();
}

void Renderer::RenderFrame(){
    DataBuffer::UpdateCommandBuffer();
}

void Renderer::DestroyRenderer(){
    Shader::Cleanup();
    Swapchain::DestroySwapchain();
    DescriptorManager::Cleanup();
    DataBuffer::Cleanup();
    ImageImpl::Cleanup();
    CommandPool::DestroyCommandPools();
    vkDestroySurfaceKHR(Instance::GetInstance(), Window::GetVulkanSurface(), nullptr);
    Device::DestroyDevice();
    Instance::DestroyInstance();
}
}
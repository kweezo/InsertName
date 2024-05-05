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
    Camera::Init();
}

void Renderer::SoftInit(){
    Swapchain::CreateSwapchain();
    Shader::Initialize();
}

void Renderer::RenderFrame(){
    DataBuffer::UpdateCommandBuffer();
    Camera::Update();
}

void Renderer::DestroyRenderer(){
    Camera::Cleanup();
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
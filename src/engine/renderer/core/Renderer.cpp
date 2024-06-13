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
    ImageImpl::Initialize();
    Camera::Init();
}

void Renderer::SoftInit(){
    Swapchain::CreateSwapchain();
    GraphicsPipeline::Init();
    Shader::Initialize();
}

void Renderer::RenderFrame(){
    DataBuffer::UpdateCommandBuffer();
    Camera::Update();
}

void Renderer::DestroyRenderer(){
    ModelInstance::Cleanup();
    Camera::Cleanup();
    Shader::Cleanup();
    Swapchain::DestroySwapchain();
    DescriptorManager::Cleanup();
    DataBuffer::Cleanup();
    ImageImpl::Cleanup();
    vkDestroySurfaceKHR(Instance::GetInstance(), Window::GetVulkanSurface(), nullptr);
    GraphicsPipeline::Cleanup();
    Device::DestroyDevice();
    Instance::DestroyInstance();
}
}
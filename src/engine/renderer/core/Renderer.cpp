#include "Renderer.hpp"

namespace renderer{

void Renderer::Init(){
    HardInit();
    SoftInit();
}
void Renderer::HardInit(){
    __Instance::Init();
    __Device::Init();
    Window::Init();
    __CommandBuffer::Init();
    __Image::Init();
    __DataBuffer::Init();
    Camera::__Init();
}

void Renderer::SoftInit(){
    __Swapchain::Init();
    __GraphicsPipeline::Init();
    ShaderManager::Init();
}

void Renderer::RenderFrame(){
    __DataBuffer::Update();
    Camera::__Update();
}

void Renderer::Cleanup(){
    ModelInstance::__Cleanup();
    ShaderManager::Cleanup();
    __Swapchain::Cleanup();
    Camera::__Cleanup();
    __DescriptorManager::Cleanup();
    __DataBuffer::Cleanup();
    __Image::Cleanup();
    vkDestroySurfaceKHR(__Instance::GetInstance(), Window::GetVulkanSurface(), nullptr);
    __GraphicsPipeline::Cleanup();
    __CommandPool::Cleanup();
    __Device::Cleanup();
    __Instance::Cleanup();
}
}
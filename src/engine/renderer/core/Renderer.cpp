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
    __Image::Init();
    Camera::__Init();
}

void Renderer::SoftInit(){
    __Swapchain::Init();
    __GraphicsPipeline::Init();
    __DataBuffer::Init();
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
    __DescriptorManager::Cleanup();
    __DataBuffer::Cleanup();
    __Image::Cleanup();
    vkDestroySurfaceKHR(__Instance::GetInstance(), Window::GetVulkanSurface(), nullptr);
    __GraphicsPipeline::Cleanup();
    __Device::Cleanup();
    __Instance::Cleanup();
}
}